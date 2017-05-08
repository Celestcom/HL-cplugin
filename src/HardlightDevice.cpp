#include "stdafx.h"
#include "HardlightDevice.h"
#include "NSLoader.h"

#include "SuitEvent.h"
#include <boost/uuid/random_generator.hpp>

class MyBasicHapticEvent : public IRetainedEvent {
public:
	MyBasicHapticEvent(CommandBuffer& commands, uint32_t area, float duration, float strength, uint32_t effect) :
		m_area(area),
		m_duration(duration),
		m_strength(strength),
		m_effect(effect),
		m_time(0),
		m_commands(commands),
		m_mutex(),
		m_playing(true) {

	}
private:
	CommandBuffer& m_commands;
	uint32_t m_area;
	float m_duration;
	float m_strength;
	uint32_t m_effect;
	float m_time;
	std::mutex m_mutex;
	bool m_playing;
	virtual void Begin() override
	{
		//we could cache the command if this ever ever ever becomes a bottleneck
		using namespace NullSpaceIPC;
		EffectCommand command;
		command.set_area(m_area);
		command.set_command(m_duration == 0.0 ? EffectCommand_Command::EffectCommand_Command_PLAY : NullSpaceIPC::EffectCommand_Command_PLAY_CONTINUOUS);
		command.set_effect(m_effect);
		command.set_strength(m_strength);

		std::lock_guard<std::mutex> guard(m_mutex);
		m_commands.push_back(std::move(command));
	}


	virtual void Pause() override
	{
		using namespace NullSpaceIPC;
		EffectCommand command;
		command.set_area(m_area);
		command.set_command(EffectCommand_Command::EffectCommand_Command_HALT);

		std::lock_guard<std::mutex> guard(m_mutex);
		m_commands.push_back(std::move(command));
		m_playing = false;
	}


	virtual void Resume() override
	{
		Begin();
	}


	virtual std::vector<NullSpaceIPC::EffectCommand> Update(float dt) override
	{
		if (m_playing) {
			m_time += dt;
		}

		std::lock_guard<std::mutex> guard(m_mutex);
		auto result = m_commands;
		m_commands.clear();
		return result;
	}


	virtual bool Finished() override
	{
		return m_time >= m_duration;
	}

};


class RetainedEventCreator : public boost::static_visitor<std::unique_ptr<IRetainedEvent>> {
private:
	//Main ID of the effect
	boost::uuids::uuid& m_id;
	CommandBuffer& m_buffer;
public:
	RetainedEventCreator(boost::uuids::uuid& id, CommandBuffer& buffer) : m_id(id), m_buffer(buffer) {}

	std::unique_ptr<IRetainedEvent> operator()(const BasicHapticEvent& hapticEvent) const {
		auto ptr = std::unique_ptr<IRetainedEvent>(new MyBasicHapticEvent(m_buffer, hapticEvent.Area, hapticEvent.Duration, hapticEvent.Strength, hapticEvent.RequestedEffectFamily));
		return ptr;
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
	chestLeft = std::make_shared<Hardlight_Mk3_ZoneDriver>();
}

void HardlightDevice::RegisterDrivers(EventRegistry& registry)
{
	registry.RegisterEventDriver("left_chest", chestLeft);
}

void HardlightDevice::UnregisterDrivers(EventRegistry& registry)
{
	//remove event drivers
}

CommandBuffer HardlightDevice::GenerateHardwareCommands(float dt)
{
	CommandBuffer result;
	auto cl = chestLeft->update(dt);
	result.insert(result.begin(), cl.begin(), cl.end());
	return result;
		
}

CommandBuffer Hardlight_Mk3_ZoneDriver::update(float dt)
{
	if (m_events.empty()) {
		return CommandBuffer();
	}
	//since m_events is a stack-like structure, we only care about returning the events of the top level item

	for (std::size_t i = 0; i < m_events.size() - 1 ; i++) {
			m_events[i].event->Update(dt);
	}
	
	return m_events.back().event->Update(dt);

}


boost::uuids::uuid Hardlight_Mk3_ZoneDriver::Id() const
{
	return m_id;
}

Hardlight_Mk3_ZoneDriver::Hardlight_Mk3_ZoneDriver() : m_id(boost::uuids::random_generator()())
{

}

void Hardlight_Mk3_ZoneDriver::createRetained(boost::uuids::uuid handle, const SuitEvent & event)
{
	CommandBuffer temp;
	auto ptr = boost::apply_visitor(RetainedEventCreator(handle, temp), event);
	//the back of our vector is always the active effect
	//it's supposed to be like a stack: the effect at the back is the active one
	m_events.push_back(GeneratedEvent(handle, std::move(ptr)));
}

void Hardlight_Mk3_ZoneDriver::controlRetained(boost::uuids::uuid handle, NSVR_PlaybackCommand command)
{

	auto it = std::find(m_events.begin(), m_events.end(), GeneratedEvent(handle, nullptr));
	if (it == m_events.end()) {
		return;
	}


	switch (command) {
	case NSVR_PlaybackCommand::NSVR_PlaybackCommand_Play:
		(*it).event->Resume();
		break;
	case NSVR_PlaybackCommand::NSVR_PlaybackCommand_Pause:
		(*it).event->Pause();
		break;
	case NSVR_PlaybackCommand::NSVR_PlaybackCommand_Reset:
		(*it).event->Pause();
		m_events.erase(it);
		break;
	default:
		break;
	}
}
