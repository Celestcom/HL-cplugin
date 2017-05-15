#include "stdafx.h"
#include "HardlightDevice.h"
#include "NSLoader.h"
#include "Locator.h"
#include "SuitEvent.h"
#include <boost/uuid/random_generator.hpp>


IRetainedEvent* ZoneModel::CurrentlyPlaying() {
		if (!m_events.empty()) {
			return m_events.back().event.get();
		}
		else {
			return nullptr;
		}
	}


void ZoneModel::Put(boost::uuids::uuid id, std::unique_ptr<IRetainedEvent> event) {
	//must determine if the previous needs to be emit the "pause" command first, conceptually and practically	

	m_events.emplace_back(id, std::move(event));
	m_events.back().event->Begin(&m_commands);
}

void ZoneModel::Remove(boost::uuids::uuid id)
{
	if (m_events.empty()) {
		return;
	}

	Pause(id);

	m_events.erase(std::remove(m_events.begin(), m_events.end(), GeneratedEvent(id, nullptr)), m_events.end());
}

void ZoneModel::Play(boost::uuids::uuid id) {
	if (m_events.empty()) {
		return;
	}
	auto it = std::find(m_events.begin(), m_events.end(), GeneratedEvent(id, nullptr));
	if (it != m_events.end()) {
		if (*it == m_events.back()) {
			(*it).event->Resume(&m_commands);
		}
		else {
			(*it).event->Resume(nullptr);
		}
	}
	
}
void ZoneModel::Pause(boost::uuids::uuid id) {
	if (m_events.empty()) {
		return;
	}

	auto it = std::find(m_events.begin(), m_events.end(), GeneratedEvent(id, nullptr));
	if (it != m_events.end()) {
		if (*it == m_events.back()) {
			(*it).event->Pause(&m_commands);
		}
		else {
			(*it).event->Pause(nullptr);
		}
	}
}

CommandBuffer ZoneModel::Update(float dt) {

	for (auto& event : m_events) {
		event.event->Update(dt);

		//if the event is finished, we should have it emit the pause commands
		if (event.event->Finished()) {
			Pause(event.id);
		}
	}

	//Remove all the finished events
	m_events.erase(std::remove_if(m_events.begin(), m_events.end(), [](const auto& event) { return event.event->Finished(); }), m_events.end());

	CommandBuffer copy;
	copy.swap(m_commands);
	std::reverse(copy.begin(), copy.end());
	return copy;

}

class MyBasicHapticEvent : public IRetainedEvent {
public:
	MyBasicHapticEvent( uint32_t area, float duration, float strength, uint32_t effect) :
		m_area(area),
		m_duration(duration),
		m_strength(strength),
		m_effect(effect),
		m_time(0),
		m_playing(true) {

	}
private:
	uint32_t m_area;
	float m_duration;
	float m_strength;
	uint32_t m_effect;
	float m_time;

	bool m_playing;


	virtual void Begin(CommandBuffer* buffer) override
	{
		m_playing = true;

		if (buffer != nullptr) {
			//we could cache the command if this ever ever ever becomes a bottleneck
			using namespace NullSpaceIPC;
			EffectCommand command;
			command.set_area(m_area);
			command.set_command(m_duration == 0.0 ? EffectCommand_Command::EffectCommand_Command_PLAY : NullSpaceIPC::EffectCommand_Command_PLAY_CONTINUOUS);
			command.set_effect(m_effect);
			command.set_strength(m_strength);


			buffer->push_back(std::move(command));
		}
	}


	virtual void Pause(CommandBuffer* buffer) override
	{
		m_playing = false;

		if (buffer != nullptr) {
			using namespace NullSpaceIPC;
			EffectCommand command;
			command.set_area(m_area);
			command.set_command(EffectCommand_Command::EffectCommand_Command_HALT);


			buffer->push_back(std::move(command));
		}
		
	}


	virtual void Resume(CommandBuffer* buffer) override
	{
		Begin(buffer);
	}


	virtual void Update(float dt) override
	{
		//note: you must call Pause to actually pause the effect
		if (m_playing) {
			m_time += dt;
		}

	
	}


	virtual bool Finished() const override
	{
		return m_time >= m_duration;
	}

};


class RetainedEventCreator : public boost::static_visitor<std::unique_ptr<IRetainedEvent>> {
private:
	//Main ID of the effect
	boost::uuids::uuid& m_id;
	uint32_t m_area;
public:
	RetainedEventCreator(boost::uuids::uuid& id, uint32_t area) : m_id(id), m_area(area) {}

	std::unique_ptr<IRetainedEvent> operator()(const BasicHapticEvent& hapticEvent) const {
		return std::unique_ptr<IRetainedEvent>(
			new MyBasicHapticEvent(m_area, hapticEvent.Duration, hapticEvent.Strength, hapticEvent.RequestedEffectFamily)
		);
		
	}
};

GeneratedEvent::GeneratedEvent(boost::uuids::uuid id, std::unique_ptr<IRetainedEvent> event) : id(id), event(std::move(event)) 
{

}

bool GeneratedEvent::operator==(const GeneratedEvent & other)
{
	
	return id == other.id;
	
}

HardlightDevice::HardlightDevice() 
{
	for (int loc = (int)Location::Lower_Ab_Right; loc != (int)Location::Error; loc++) {
		m_drivers.push_back(std::make_shared<Hardlight_Mk3_ZoneDriver>(Location(loc)));
	}
	
	//	m_drivers.push_back(std::make_shared<Hardlight_Mk3_ZoneDriver>(Location::Lower_Ab_Right));
	//}

}

void HardlightDevice::RegisterDrivers(EventRegistry& registry)
{
	auto& translator = Locator::getTranslator();
	for (auto& driver : m_drivers) {
		auto region = translator.ToRegionString(
			translator.ToArea(driver->Location())
		);

		registry.RegisterEventDriver(region, driver);
		registry.RegisterRtpDriver(region, driver);
	}
	
}

void HardlightDevice::UnregisterDrivers(EventRegistry& registry)
{
	//remove event drivers

}

CommandBuffer HardlightDevice::GenerateHardwareCommands(float dt)
{
	CommandBuffer result;
	for (auto& driver : m_drivers) {
		auto cl = driver->update(dt);
		result.insert(result.begin(), cl.begin(), cl.end());
	}
	return result;
		
}

CommandBuffer Hardlight_Mk3_ZoneDriver::update(float dt)
{
	//think about if the commandbuffer vectors should really be reversed
	auto rtpCommands = m_rtpModel.Update(dt);
	auto retainedCommands = m_retainedModel.Update(dt);
	
	std::lock_guard<std::mutex> guard(m_mutex);

	CommandBuffer result;
	result.swap(m_commands);

	if (m_currentMode == Mode::Realtime) {
		result.insert(result.end(), rtpCommands.begin(), rtpCommands.end());
	}
	else {
		result.insert(result.end(), retainedCommands.begin(), retainedCommands.end());
	}

	return result;

}


boost::uuids::uuid Hardlight_Mk3_ZoneDriver::Id() const
{
	return m_id;
}


Hardlight_Mk3_ZoneDriver::Hardlight_Mk3_ZoneDriver(::Location area) : 
	m_id(boost::uuids::random_generator()()), 
	m_area(static_cast<uint32_t>(area)),
	m_currentMode(Mode::Retained),
	m_commands(),
	m_rtpModel(m_area),
	m_retainedModel(),
	m_mutex()
{

}

::Location Hardlight_Mk3_ZoneDriver::Location()
{
	return static_cast<::Location>(m_area);
}

void Hardlight_Mk3_ZoneDriver::realtime(const RealtimeArgs& args)
{
	m_rtpModel.ChangeVolume(args.volume);
	transitionInto(Mode::Realtime);
}

void Hardlight_Mk3_ZoneDriver::transitionInto(Mode mode)
{
	std::lock_guard<std::mutex> guard(m_mutex);

	using namespace NullSpaceIPC;
	if (m_currentMode == Mode::Realtime) {
		if (mode == Mode::Retained) { 
			m_currentMode = Mode::Retained;
			//send command to go into retained mode.
			EffectCommand enableRetained;
			enableRetained.set_area(m_area);
			enableRetained.set_command(NullSpaceIPC::EffectCommand_Command_ENABLE_INTRIG);
			m_commands.push_back(enableRetained);
		}
	}
	else {
		if (mode == Mode::Realtime) {
			m_currentMode = Mode::Realtime;
			EffectCommand enableRetained;
			enableRetained.set_area(m_area);
			enableRetained.set_command(NullSpaceIPC::EffectCommand_Command_ENABLE_RTP);
			m_commands.push_back(enableRetained);
		}
	}
}

void Hardlight_Mk3_ZoneDriver::createRetained(boost::uuids::uuid handle, const SuitEvent & event)
{

	auto retainedEvent = boost::apply_visitor(RetainedEventCreator(handle, m_area), event);
	m_retainedModel.Put(handle, std::move(retainedEvent));

	transitionInto(Mode::Retained);

}

void Hardlight_Mk3_ZoneDriver::controlRetained(boost::uuids::uuid handle, NSVR_PlaybackCommand command)
{

	switch (command) {
	case NSVR_PlaybackCommand::NSVR_PlaybackCommand_Play:
		m_retainedModel.Play(handle);
		break;
	case NSVR_PlaybackCommand::NSVR_PlaybackCommand_Pause:
		m_retainedModel.Pause(handle);
		break;
	case NSVR_PlaybackCommand::NSVR_PlaybackCommand_Reset:
		m_retainedModel.Remove(handle);
	default:
		break;
	}
}

RtpModel::RtpModel(uint32_t area) :m_area(area), m_volume(0), m_commands()
{
}

void RtpModel::ChangeVolume(int newVolume)
{
	if (newVolume != m_volume) {
		m_volume = newVolume;

		using namespace NullSpaceIPC;
		EffectCommand command;
		command.set_command(EffectCommand_Command::EffectCommand_Command_PLAY_RTP);
		command.set_strength(255-(m_volume / 255.0f));
		command.set_area(m_area);
		
		std::lock_guard<std::mutex> guard(m_mutex);
		m_commands.push_back(std::move(command));

	}
}

CommandBuffer RtpModel::Update(float dt)
{
	CommandBuffer copy;
	std::lock_guard<std::mutex> guard(m_mutex);

	copy.swap(m_commands);
	std::reverse(copy.begin(), copy.end());
	return copy;

}
