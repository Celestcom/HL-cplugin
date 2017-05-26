#include "stdafx.h"
#include "HardlightDevice.h"
#include "NSLoader.h"
#include "Locator.h"
#include "SuitEvent.h"
#include <boost/uuid/random_generator.hpp>



MyBasicHapticEvent::MyBasicHapticEvent(boost::uuids::uuid parent_id, boost::uuids::uuid unique_id, uint32_t area, float duration, float strength, uint32_t effect) :
		m_parentId(parent_id),
		m_uniqueId(unique_id),
		m_area(area),
		m_duration(duration),
		m_strength(strength),
		m_effect(effect),
		m_time(0),
		m_playing(true) {
}

MyBasicHapticEvent::MyBasicHapticEvent(boost::uuids::uuid id) : m_parentId(id) {}


bool MyBasicHapticEvent::operator==(const MyBasicHapticEvent & other) const
{

	return m_uniqueId == other.m_uniqueId;

}



bool MyBasicHapticEvent::IsFunctionallyIdentical(const MyBasicHapticEvent &other)
{
	return m_area == other.m_area
		&& m_duration == other.m_duration
		&& m_effect == other.m_effect
		&& m_strength == other.m_strength;
}

CommandBuffer MyBasicHapticEvent::EmitCleanupCommands() const
{
	//if (m_duration == 0.0) {
	//	return CommandBuffer();
	//}

	
	using namespace NullSpaceIPC;

	EffectCommand command;
	command.set_area(m_area);
	command.set_command(EffectCommand_Command::EffectCommand_Command_HALT);

	CommandBuffer result;
	result.push_back(std::move(command));
	return result;

}


CommandBuffer MyBasicHapticEvent::EmitCreationCommands() const{

	//we could cache the command if this ever ever ever becomes a bottleneck
	using namespace NullSpaceIPC;
	EffectCommand command;
	command.set_area(m_area);
	command.set_command(m_duration == 0.0 ? EffectCommand_Command::EffectCommand_Command_PLAY : NullSpaceIPC::EffectCommand_Command_PLAY_CONTINUOUS);
	command.set_effect(m_effect);
	command.set_strength(m_strength);

	CommandBuffer result;
	result.push_back(std::move(command));
	return result;
	
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



//aquires the events mutex
void ZoneModel::Put(MyBasicHapticEvent event) {

	std::lock_guard<std::mutex> guard(m_stagingLock);
	m_stagingEvents.push_back(std::move(event));
	//std::lock_guard<std::mutex> guard(m_eventsMutex);

	//setCreationCommands(event.EmitCreationCommands());

	//if (m_events.empty()) {
	//	if (isOneshot(event)) {
	//		setCleanupCommands(event.EmitCleanupCommands());
	//	}
	//	m_events.push_back(std::move(event));
	//}
	//
	//else if (isOneshot(m_events.back())) {
	//	//We want newer oneshots to replace older oneshots
	//	//We want a continuous play to replace a oneshot
	//	std::swap(m_events.back(), event);
	//}

	//else if (isContinuous(m_events.back()) && isOneshot(event)) {
	//	//We want a oneshot to play over a continuous
	//	//But the firmware requires a dirty hack: send a halt first
	//	setCleanupCommands(m_events.back().EmitCleanupCommands());
	//	m_events.push_back(std::move(event));
	//} 

	//else {
	//	//Continuous should be added over continuous
	//	m_events.push_back(std::move(event));
	//}
}


struct parent_id_matches {
	parent_id_matches(boost::uuids::uuid id) : id(id) {}
	boost::uuids::uuid id;
	bool operator()(const MyBasicHapticEvent& event) const {
		return event.m_parentId == id;
	}
};


struct unique_id_matches {
	unique_id_matches(const MyBasicHapticEvent& event) : id(event.m_uniqueId) {}
	boost::uuids::uuid id;
	bool operator()(const MyBasicHapticEvent& event) const {
		return event.m_uniqueId == id;
	}
};

//acquires events and paused mutexes
void ZoneModel::Remove(boost::uuids::uuid id)
{

	std::lock_guard<std::mutex> guard(m_stagingLock);
	m_stagingCommands.emplace_back(id, UserCommand::Command::Remove);
	//First we pause for the side effects of stopping the event
	//Pause(id);
	//
	//{
	//	std::lock_guard<std::mutex> guard(m_pausedMutex);
	//	//Second we remove from paused (because pause sticks stuff in the paused list)
	//	m_pausedEvents.erase(std::remove_if(m_pausedEvents.begin(), m_pausedEvents.end(), parent_id_matches(id)), m_pausedEvents.end());

	//}
}

//acquires the paused and events mutexes
void ZoneModel::Play(boost::uuids::uuid id) {


	std::lock_guard<std::mutex> guard(m_stagingLock);
	m_stagingCommands.emplace_back(id, UserCommand::Command::Play);


	//std::lock_guard<std::mutex> guard(m_pausedMutex);

	//if (m_pausedEvents.empty()) {
	//	return;
	//}

	//for (const auto& event : m_pausedEvents) {
	//	if (parent_id_matches(id)(event)) {
	//		Put(event);
	//	}
	//}

	//m_pausedEvents.erase(std::remove_if(m_pausedEvents.begin(), m_pausedEvents.end(), parent_id_matches(id)), m_pausedEvents.end());
}

const ZoneModel::PausedContainer& ZoneModel::PausedEvents()
{
	return m_pausedEvents;
}

const ZoneModel::PlayingContainer& ZoneModel::PlayingEvents()
{
	return m_events;
}

void ZoneModel::swapOutEvent(const MyBasicHapticEvent& event) {
	assert(!m_events.empty());

	//is it the only event in the stack?
	if (m_events.size() == 1) {
		if (isContinuous(event)) {
			setCleanupCommands(event.EmitCleanupCommands());
		}
	}
	else {
		if (isOneshot(event)) {
			//if swapping out a oneshot..
		}
		const auto& newTop = m_events.end()[-2];
		assert(isContinuous(newTop)); //should never have the hack case of going from cont to oneshot
		setCreationCommands(newTop.EmitCreationCommands());
	}

	
}

bool ZoneModel::isTopEvent(const MyBasicHapticEvent& event) const {
	assert(!m_events.empty());
	return event == m_events.back();
}

//acquires the events and paused mutexes
void ZoneModel::Pause(boost::uuids::uuid id) {
	std::lock_guard<std::mutex> guard(m_stagingLock);
	m_stagingCommands.emplace_back(id, UserCommand::Command::Pause);



	/*
	Case 0: There are no events
		Return
	Case 1: There are no matching events
		Return 
	Case 2: The matching events contain an event which is at the top of the stack (i.e. playing)
		Clean up that effect
		Put all effects in paused
		Remove all effects from playing
	Case 3: The matching events do not contain an event which is at the top of the stack
		Put all effects in paused
		remove all effects from playing
	*/

	//std::lock_guard<std::mutex> guard(m_eventsMutex);

	////Case 0
	//if (m_events.empty()) {
	//	return;
	//}

	////PROBLEM IS, PARENT MATCH ID WONT MATCH ON UNIQUE ID DUHHHH
	////Case 1
	//std::vector<MyBasicHapticEvent> matchedEvents;
	//std::copy_if(m_events.begin(), m_events.end(), std::back_inserter(matchedEvents), parent_id_matches(id));

	//if (matchedEvents.empty()) {
	//	return;
	//}

	//auto playingEvent = std::find_if(matchedEvents.begin(), matchedEvents.end(), unique_id_matches(m_events.back()));

	////Case 2
	//if (playingEvent != matchedEvents.end()) {
	//	swapOutEvent(*playingEvent);
	//}

	////Case 3 & tail of Case 2

	//{
	//	std::lock_guard<std::mutex> guard(m_pausedMutex);
	//	//First copy all the matched events into paused
	//	m_pausedEvents.insert(m_pausedEvents.end(), matchedEvents.begin(), matchedEvents.end());
	//}

	////Then erase them from playing
	//m_events.erase(std::remove_if(m_events.begin(), m_events.end(), parent_id_matches(id)), m_events.end());

	//
}

void pruneOneshots(std::vector<MyBasicHapticEvent>* events) {
	auto secondToLast = events->end() - 1;
	events->erase(std::remove_if(events->begin(), secondToLast, isOneshot), secondToLast);
}
CommandBuffer ZoneModel::Update(float dt) {

	m_stagingLock.lock();
	auto requestedEvents = m_stagingEvents;
	auto requestedCommands = m_stagingCommands;
	m_stagingLock.unlock();


	//First off, remove unreachable oneshots that will never play.
	//This means every oneshot that is not at the back of the list will be removed.
	pruneOneshots(&requestedEvents);

	//Second, add all the events to the model
	std::copy(requestedEvents.begin(), requestedEvents.end(), m_events.begin())









	//{
	//	std::lock_guard<std::mutex> guard(m_eventsMutex);
	//	
	//	auto event = m_events.begin();
	//	while (event != m_events.end()) {
	//		event->Update(dt);
	//		if (event->Finished()) {
	//			if (isTopEvent(*event)) { swapOutEvent(*event); }
	//			event = m_events.erase(event);
	//		}
	//		else {
	//			++event;
	//		}
	//	}
	//}
	//

	//
	//CommandBuffer results;

	//{
	//	std::lock_guard<std::mutex> guard(m_commandsMutex);
	//	results.insert(results.end(), m_cleanupCommands.begin(), m_cleanupCommands.end());
	//	results.insert(results.end(), m_creationCommands.begin(), m_creationCommands.end());
	//	m_cleanupCommands.clear();
	//	m_creationCommands.clear();
	//}

	//

	//return results;
	return CommandBuffer();
}

ZoneModel::ZoneModel(): 
	m_cleanupCommands(), 
	m_creationCommands(), 
	m_pausedEvents(),  
	m_events(),
	m_eventsMutex(),
	m_commandsMutex(),
	m_pausedMutex()
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

void ZoneModel::setCreationCommands(CommandBuffer buffer)
{
	std::lock_guard<std::mutex> guard(m_commandsMutex);
	m_creationCommands = buffer;
}

void ZoneModel::setCleanupCommands(CommandBuffer buffer)
{
	std::lock_guard<std::mutex> guard(m_commandsMutex);
	m_cleanupCommands = buffer;
}



class BasicHapticEventCreator : public boost::static_visitor<MyBasicHapticEvent> {
private:
	//Main ID of the effect
	boost::uuids::uuid m_parentId;
	uint32_t m_area;
	boost::uuids::uuid m_uniqueId;
public:
	BasicHapticEventCreator(boost::uuids::uuid id, boost::uuids::uuid unique_id, uint32_t area) : m_parentId(id), m_uniqueId(unique_id), m_area(area) {}

	MyBasicHapticEvent operator()(const BasicHapticEvent& hapticEvent) const {
		return MyBasicHapticEvent(m_parentId, m_uniqueId, m_area, hapticEvent.Duration, hapticEvent.Strength, hapticEvent.RequestedEffectFamily);
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
	return m_parentId;
}


Hardlight_Mk3_ZoneDriver::Hardlight_Mk3_ZoneDriver(::Location area) : 
	m_parentId(boost::uuids::random_generator()()), 
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

	m_retainedModel.Put(boost::apply_visitor(BasicHapticEventCreator(handle, m_gen(), m_area), event));

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

ZoneModel::UserCommand::UserCommand(boost::uuids::uuid id, Command c) : id(id), command(c)
{
}
