#include "stdafx.h"
#include "HardlightDevice.h"
#include "NSLoader.h"
#include "Locator.h"
#include "SuitEvent.h"
#include <boost/uuid/random_generator.hpp>

#include <experimental/vector>
void ZoneModel::Put(LiveBasicHapticEvent event) {
	incomingEvents.push(std::move(event));
}

void ZoneModel::Remove(boost::uuids::uuid id) {
	incomingCommands.push(UserCommand(id, UserCommand::Command::Remove));
}

void ZoneModel::Play(boost::uuids::uuid id) {
	incomingCommands.push(UserCommand(id, UserCommand::Command::Play));
}

void ZoneModel::Pause(boost::uuids::uuid id) {
	incomingCommands.push(UserCommand(id, UserCommand::Command::Pause));
}

const ZoneModel::PausedContainer& ZoneModel::PausedEvents() {
	return pausedEvents;
}

const ZoneModel::PlayingContainer& ZoneModel::PlayingEvents() {
	return playingEvents;
}


CommandBuffer ZoneModel::Update(float dt) {

	updateExistingEvents(dt);
	removeExpiredEvents();

	handleNewEvents();
	handleNewCommands();

	return generateCommands();
}



void ZoneModel::updateExistingEvents(float dt)
{
	for (auto& event : playingEvents) {
		event.update(dt);
	}
}


void ZoneModel::removeExpiredEvents() {
	auto expired = [](auto& e) {
		return e.isFinished();
	};

	auto toRemove = std::remove_if(playingEvents.begin(), playingEvents.end(), expired);
	playingEvents.erase(toRemove, playingEvents.end());
}

template<class T>
std::vector<T> consumeAll(boost::lockfree::spsc_queue<T>& queue) {
	std::vector<T> result = std::vector<T>();
	queue.consume_all([&](auto element) {
		result.push_back(element);
	});
	return result;
}


void ZoneModel::handleNewCommands() {
	std::vector<UserCommand> newCommands = consumeAll(incomingCommands);
	for (const auto& command : newCommands) {
		switch (command.command) {
		case UserCommand::Command::Play:
			resumeAllChildren(command.id);
			break;
		case UserCommand::Command::Pause:
			pauseAllChildren(command.id);
			break;
		case UserCommand::Command::Remove:
			removeAllChildren(command.id);
			break;
		default:
			break;
		}
	}
}


void pruneOneshots(std::vector<LiveBasicHapticEvent>* events) {
	if (events->empty()) {
		return;
	}

	auto isOneshot = [](const LiveBasicHapticEvent& e) {return e.isOneshot(); };

	auto secondToLast = events->end() - 1;

	auto toRemove = std::remove_if(events->begin(), secondToLast, isOneshot);
	events->erase(toRemove, secondToLast);
}




void ZoneModel::handleNewEvents() {
	std::vector<LiveBasicHapticEvent> newlyArrivedEvents = consumeAll(incomingEvents);

	pruneOneshots(&newlyArrivedEvents);

	playingEvents.insert(playingEvents.end(), newlyArrivedEvents.begin(), newlyArrivedEvents.end());
}





CommandBuffer ZoneModel::generateCommands()
{

	if (playingEvents.empty()) {
		return stateChanger.transitionToIdle();
	}
	else {
		return stateChanger.transitionTo(playingEvents.back());

	}
}

ZoneModel::ZoneModel(uint32_t area):
	pausedEvents(),  
	playingEvents(),
	stateChanger(area),
	incomingEvents(512),
	incomingCommands(512)
{ 

}

template<class T, class Predicate>
void copy_then_remove_if(std::vector<T>& from, std::vector<T>& to, Predicate predicate) {
	std::copy_if(
		from.begin(),
		from.end(),
		std::back_inserter(to),
		predicate
	);

	std::experimental::erase_if(from, predicate);
}


struct is_child_of {
	boost::uuids::uuid id;
	is_child_of(boost::uuids::uuid id) : id(id) {}
	bool operator()(const LiveBasicHapticEvent& e) {
		return e.isChildOf(id);
	}
};

void ZoneModel::pauseAllChildren(const boost::uuids::uuid & id)
{
	copy_then_remove_if(playingEvents, pausedEvents, is_child_of(id));
}


void ZoneModel::resumeAllChildren(const boost::uuids::uuid & id)
{
	copy_then_remove_if(pausedEvents, playingEvents, is_child_of(id));
}

void ZoneModel::removeAllChildren(const boost::uuids::uuid & id)
{
	std::experimental::erase_if(playingEvents, is_child_of(id));
	std::experimental::erase_if(pausedEvents, is_child_of(id));
}



class BasicHapticEventCreator : public boost::static_visitor<LiveBasicHapticEvent> {
private:
	//Main ID of the effect
	boost::uuids::uuid m_parentId;
	uint32_t m_area;
	boost::uuids::uuid m_uniqueId;
public:
	BasicHapticEventCreator(boost::uuids::uuid id, boost::uuids::uuid unique_id, uint32_t area) : m_parentId(id), m_uniqueId(unique_id), m_area(area) {}

	LiveBasicHapticEvent operator()(const BasicHapticEvent& hapticEvent) const {
		BasicHapticEventData data;
		data.area = m_area;
		data.duration = hapticEvent.Duration;
		data.strength = hapticEvent.Strength;
		data.effect = hapticEvent.RequestedEffectFamily;
		return LiveBasicHapticEvent(m_parentId, m_uniqueId, data);
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
		CommandBuffer cl = driver->update(dt);
		//std::reverse(cl.begin(), cl.end()); todo: see if necessary order is already created
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
	m_retainedModel(m_area),
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
		command.set_strength((m_volume/2) / 128.0f);
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

ZoneModel::UserCommand::UserCommand(): id(), command(Command::Unknown)
{
}

ZoneModel::UserCommand::UserCommand(boost::uuids::uuid id, Command c) : id(id), command(c)
{
}

LiveBasicHapticEvent::LiveBasicHapticEvent() :
	parentId(), 
	uniqueId(), 
	currentTime(0), 
	isPlaying(false), 
	eventData()
{
}

LiveBasicHapticEvent::LiveBasicHapticEvent(boost::uuids::uuid parentId, boost::uuids::uuid uniqueId, BasicHapticEventData data):
	parentId(parentId),
	uniqueId(uniqueId),
	currentTime(0),
	isPlaying(true),
	eventData(std::move(data))

{

}

const BasicHapticEventData & LiveBasicHapticEvent::Data() const
{
	return eventData;
}

bool LiveBasicHapticEvent::operator==(const LiveBasicHapticEvent & other) const
{
	return uniqueId == other.uniqueId;
}

void LiveBasicHapticEvent::update(float dt)
{
	if (isPlaying) {
		currentTime += dt;
	}
}

bool LiveBasicHapticEvent::isFinished() const
{
	return currentTime >= eventData.duration;
}

bool LiveBasicHapticEvent::isContinuous() const
{
	return !isOneshot();
}

bool LiveBasicHapticEvent::isOneshot() const
{
	return eventData.duration == 0.0;
}

bool LiveBasicHapticEvent::isChildOf(const boost::uuids::uuid & parentId) const
{
	return this->parentId == parentId;
}

boost::uuids::uuid LiveBasicHapticEvent::GetParentId()
{
	return parentId;
}

MotorStateChanger::MotorStateChanger(uint32_t areaId):
	currentState(MotorFirmwareState::Idle),
	previousContinuous(),
	area(areaId)
{
}

MotorStateChanger::MotorFirmwareState MotorStateChanger::GetState() const
{
	return currentState;
}

CommandBuffer MotorStateChanger::transitionTo(const LiveBasicHapticEvent & event)
{
	CommandBuffer commands;
	if (event == previousContinuous) {
		commands = CommandBuffer();
	} else if (event.isOneshot()) {

		commands = transitionToOneshot(event.Data());
		previousContinuous = boost::optional<LiveBasicHapticEvent>();

	}
	else {
		commands = transitionToContinuous(event.Data());
		previousContinuous = event;

	}

	return commands;
}

CommandBuffer MotorStateChanger::transitionToIdle()
{
	CommandBuffer requiredCmds;
	switch (currentState) {
	case MotorFirmwareState::Idle:
		//do nothing
		break;
	case MotorFirmwareState::PlayingOneshot:
		//do nothing;
		break;
	case MotorFirmwareState::PlayingContinuous:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateHalt(area));
		break;
	}

	currentState = MotorFirmwareState::Idle;
	previousContinuous = boost::optional<LiveBasicHapticEvent>();
	return requiredCmds;
}

CommandBuffer MotorStateChanger::transitionToOneshot(BasicHapticEventData data)
{
	//Note: as you can see, we generate the same commands from every state.
	//This is subject to change with future firmware versions, which is why it is 
	//setup like this.

	CommandBuffer requiredCmds;
	switch (currentState) {
	case MotorFirmwareState::Idle:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateHalt(area));
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateOneshotPlay(data));
		break;
	case MotorFirmwareState::PlayingOneshot:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateHalt(area));
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateOneshotPlay(data));
		break;
	case MotorFirmwareState::PlayingContinuous:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateHalt(area));
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateOneshotPlay(data));
		break;
	}

	currentState = MotorFirmwareState::PlayingOneshot;
	return requiredCmds;
}

CommandBuffer MotorStateChanger::transitionToContinuous(BasicHapticEventData data)
{
	//Note: as you can see, we generate the same commands from every state.
	//This is subject to change with future firmware versions, which is why it is 
	//setup like this.

	CommandBuffer requiredCmds;
	switch (currentState) {
	case MotorFirmwareState::Idle:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateContinuousPlay(data));
		break;
	case MotorFirmwareState::PlayingOneshot:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateContinuousPlay(data));
		break;
	case MotorFirmwareState::PlayingContinuous:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateContinuousPlay(data));
		break;
	}

	currentState = MotorFirmwareState::PlayingContinuous;
	return requiredCmds;
}

NullSpaceIPC::EffectCommand Hardlight_Mk3_Firmware::generateContinuousPlay(const BasicHapticEventData & data)
{
	using namespace NullSpaceIPC;
	EffectCommand command;
	command.set_area(data.area);
	command.set_command(NullSpaceIPC::EffectCommand_Command_PLAY_CONTINUOUS);
	command.set_effect(data.effect);
	command.set_strength(data.strength);
	
	return command;
}

NullSpaceIPC::EffectCommand Hardlight_Mk3_Firmware::generateOneshotPlay(const BasicHapticEventData & data)
{
	using namespace NullSpaceIPC;
	EffectCommand command;
	command.set_area(data.area);
	command.set_command(NullSpaceIPC::EffectCommand_Command_PLAY);
	command.set_effect(data.effect);
	command.set_strength(data.strength);

	return command;
}

NullSpaceIPC::EffectCommand Hardlight_Mk3_Firmware::generateHalt(uint32_t area)
{
	using namespace NullSpaceIPC;
	EffectCommand command;
	command.set_area(area);
	command.set_command(NullSpaceIPC::EffectCommand_Command_HALT);
	return command;
}
