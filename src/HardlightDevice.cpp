#include "stdafx.h"
#include "HardlightDevice.h"
#include "NSLoader.h"
#include "Locator.h"
#include "SuitEvent.h"
#include <boost/uuid/random_generator.hpp>



MyBasicHapticEvent::MyBasicHapticEvent(boost::uuids::uuid id, uint32_t area, float duration, float strength, uint32_t effect) :
		m_id(id),
		m_area(area),
		m_duration(duration),
		m_strength(strength),
		m_effect(effect),
		m_time(0),
		m_playing(true) {
}

MyBasicHapticEvent::MyBasicHapticEvent(boost::uuids::uuid id) : m_id(id) {}


bool MyBasicHapticEvent::operator==(const MyBasicHapticEvent & other)
{

	return m_id == other.m_id;

}



bool MyBasicHapticEvent::IsFunctionallyIdentical(const MyBasicHapticEvent &other)
{
	return m_area == other.m_area
		&& m_duration == other.m_duration
		&& m_effect == other.m_effect
		&& m_strength == other.m_strength;
}

void MyBasicHapticEvent::EmitCleanupCommands(CommandBuffer * buffer) const
{
	if (m_duration == 0.0) {
		return;
	}

	if (buffer != nullptr) {
		using namespace NullSpaceIPC;
		EffectCommand command;
		command.set_area(m_area);
		command.set_command(EffectCommand_Command::EffectCommand_Command_HALT);


		buffer->push_back(std::move(command));
	}
}


void MyBasicHapticEvent::EmitCreationCommands(CommandBuffer* buffer) const{
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


void MyBasicHapticEvent::LogicalPlay() {
	m_playing = true;
}

void MyBasicHapticEvent::LogicalPause() {
	m_playing = false;
}


void MyBasicHapticEvent::Update(float dt)
{
	//note: you must call Pause to actually pause the effect
	if (m_playing) {
		m_time += dt;
	}


}

bool MyBasicHapticEvent::Finished() const
{
	return m_time >= m_duration;
}



inline bool isContinuous(const MyBasicHapticEvent& event) {
	return event.m_duration != 0;
}

inline bool isOneshot(const MyBasicHapticEvent& event) {
	return !isContinuous(event);
}


//these purely emit commands. They do not change the execution state of the events.
//We must do this at a higher level where we know the context, e.g. is it pausing or deleting?
CommandBuffer transitionUp(const MyBasicHapticEvent& topOfStack, const MyBasicHapticEvent& newEvent) {

	CommandBuffer result;
	newEvent.EmitCreationCommands(&result);
	return result;
}

CommandBuffer transitionDown(const MyBasicHapticEvent& topOfStack, const MyBasicHapticEvent& newEvent) {
	CommandBuffer result;
	newEvent.EmitCreationCommands(&result);
	return result;
}

//todo: Need to protect with mutexes
void ZoneModel::Put(MyBasicHapticEvent event) {

	if (m_events.empty()) {
		m_events.push_back(std::move(event));

		setCreationCommands([&](CommandBuffer* buffer) {
			m_events.back().EmitCreationCommands(buffer);
		});
	}
	else {

		auto& topOfStack = m_events.back();

		setCreationCommands([&](CommandBuffer* buffer) {
			CommandBuffer commands = transitionUp(topOfStack, event);
			buffer->insert(buffer->begin(), commands.begin(), commands.end());
		});

		if (isOneshot(topOfStack)) {
			//if it's a oneshot on top of the stack, we want any newer oneshots in this batch to replace it
			//additionally, we want any new continuous plays to replace it. This behavior may change. 
			std::swap(topOfStack, event);
		}
		else if (isContinuous(topOfStack) && isOneshot(event)) {
			//If a continuous was previously playing and we want to replace it with a oneshot,
			//we need a dirty hack to halt the cont-play. Firmware should just replace it, but it doesnt.
			setCleanupCommands([&](CommandBuffer* buffer) {
				topOfStack.EmitCleanupCommands(buffer);
			});

			m_events.push_back(std::move(event));
		}

		else {
			m_events.push_back(std::move(event));
		}
	}
}

ZoneModel::PlayingContainer::iterator ZoneModel::Remove(boost::uuids::uuid id)
{
	auto nextEvent = Pause(id);
	
	//Since pausing will move the effect to the paused list, we should actually remove it from there as well
	auto maybePausedEvent = findPausedEvent(id);
	if (maybePausedEvent != m_pausedEvents.end()) {
		m_pausedEvents.erase(maybePausedEvent);
	}

	return nextEvent;
}

void ZoneModel::Play(boost::uuids::uuid id) {
	if (m_pausedEvents.empty()) {
		return;
	}
	auto it = std::find(m_pausedEvents.begin(), m_pausedEvents.end(),MyBasicHapticEvent(id));


	if (it != m_pausedEvents.end()) {
		Put(*it);
		m_pausedEvents.erase(it);
	}
	
}
const ZoneModel::PausedContainer& ZoneModel::PausedEvents()
{
	return m_pausedEvents;
}

const ZoneModel::PlayingContainer& ZoneModel::PlayingEvents()
{
	return m_events;
}



ZoneModel::PlayingContainer::iterator ZoneModel::Pause(boost::uuids::uuid id) {
	
	/*
	if it's the top of the stack, we need to clean the effect up and put it in paused, then remove from playing.
	If at that point playing has something left, we should restart it. If it's not on the top of the stack, we
	can just remove it and put it in paused.
	*/

	if (m_events.empty()) {
		return m_events.end();
	}
	
	auto potentialEvent = findPlayingEvent(id);
	if (potentialEvent == m_events.end()) {
		return m_events.end();
	}
	//if the event exists in playing, we should put a copy of it into the paused event container
	m_pausedEvents.push_back(*potentialEvent);

	if (*potentialEvent == m_events.back()) {
		//if the event was the 'top' of the stack, we should clean it up
		if (m_events.size() > 1) {
			assert(isContinuous(*(m_events.end() - 2)));
			//since is something below it, we should create it
			setCreationCommands([&](CommandBuffer* buffer) {
				(m_events.end() - 2)->EmitCreationCommands(buffer);
			});
		}
		else {
			//since it's the only thing, we need to cleanup
			if (isContinuous(*potentialEvent)) {
				setCleanupCommands([&](CommandBuffer* buffer) {
					(*potentialEvent).EmitCleanupCommands(buffer);
				});
			}
		}

		//then remove it
		auto nextEvent = m_events.erase(potentialEvent);

		//and if there was something beneath it, tell it to create itself
		if (!m_events.empty()) {
			setCreationCommands([&](CommandBuffer* buffer) {
				m_events.back().EmitCreationCommands(buffer);
			});
		}

		return nextEvent;
	}
	else {
		//if it wasn't the top, then it has no effect on the motor, so just erase it
		return m_events.erase(potentialEvent);
	}
}

CommandBuffer ZoneModel::Update(float dt) {
	
	std::lock_guard<std::mutex> guard(m_mutex);

	auto event = m_events.begin();
	while (event != m_events.end()) {
		event->Update(dt);
		if (event->Finished()) {
			event = Remove(event->m_id);
		}
		else {
			++event;
		}
	}

	CommandBuffer results;
	results.reserve(m_cleanupCommands.size() + m_creationCommands.size());

	results.insert(results.end(), m_cleanupCommands.begin(), m_cleanupCommands.end());
	results.insert(results.end(), m_creationCommands.begin(), m_creationCommands.end());
	
	m_cleanupCommands.clear();
	m_creationCommands.clear();
	return results;

}

ZoneModel::ZoneModel(): m_cleanupCommands(), m_creationCommands(), m_pausedEvents(), m_mutex(), m_events()
{

}

ZoneModel::PlayingContainer::iterator ZoneModel::findPlayingEvent(const boost::uuids::uuid & id)
{
	return std::find(m_events.begin(), m_events.end(), MyBasicHapticEvent(id));
}

ZoneModel::PausedContainer::iterator ZoneModel::findPausedEvent(const boost::uuids::uuid& id)
{
	return std::find(m_pausedEvents.begin(), m_pausedEvents.end(), MyBasicHapticEvent(id));
}

void ZoneModel::setCreationCommands(std::function<void(CommandBuffer*buffer)> creator )
{
	m_creationCommands.clear();
	creator(&m_creationCommands);
}

void ZoneModel::setCleanupCommands(std::function<void(CommandBuffer*buffer)> cleaner)
{
	m_cleanupCommands.clear();
	cleaner(&m_cleanupCommands);
}



class BasicHapticEventCreator : public boost::static_visitor<MyBasicHapticEvent> {
private:
	//Main ID of the effect
	boost::uuids::uuid& m_id;
	uint32_t m_area;
public:
	BasicHapticEventCreator(boost::uuids::uuid& id, uint32_t area) : m_id(id), m_area(area) {}

	MyBasicHapticEvent operator()(const BasicHapticEvent& hapticEvent) const {
		return MyBasicHapticEvent(m_id, m_area, hapticEvent.Duration, hapticEvent.Strength, hapticEvent.RequestedEffectFamily);
	}
};




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

	m_retainedModel.Put(boost::apply_visitor(BasicHapticEventCreator(handle, m_area), event));

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
