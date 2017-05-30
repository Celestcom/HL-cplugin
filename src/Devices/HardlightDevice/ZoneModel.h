#pragma once

#include <boost/lockfree/spsc_queue.hpp>
#include "Enums.h"
#include "LiveBasicHapticEvent.h"
#include "IHapticDevice.h"
#include "MotorStateChanger.h"
class ZoneModel {
public:

	ZoneModel(Location area);

	void Put(LiveBasicHapticEvent event);
	void Remove(boost::uuids::uuid id);
	void Play(boost::uuids::uuid id);
	void Pause(boost::uuids::uuid id);


	typedef std::vector<LiveBasicHapticEvent> PlayingContainer;
	typedef std::vector<LiveBasicHapticEvent> PausedContainer;

	const PausedContainer& PausedEvents();
	const PlayingContainer& PlayingEvents();

	CommandBuffer Update(float dt);

	boost::optional<LiveBasicHapticEvent> GetCurrentlyPlayingEvent();


private:
	class UserCommand {
	public:
		enum class Command {
			Unknown = 0, Play = 1, Pause = 2, Remove = 3
		};
		boost::uuids::uuid id;
		Command command;
		UserCommand();
		UserCommand(boost::uuids::uuid id, Command c);
	};
	PlayingContainer playingEvents;
	PausedContainer pausedEvents;

	boost::lockfree::spsc_queue<LiveBasicHapticEvent> incomingEvents;
	boost::lockfree::spsc_queue<UserCommand> incomingCommands;

	MotorStateChanger stateChanger;

	void pauseAllChildren(const boost::uuids::uuid& id);
	void resumeAllChildren(const boost::uuids::uuid& id);
	void removeAllChildren(const boost::uuids::uuid& id);

	CommandBuffer generateCommands();
	void updateExistingEvents(float dt);
	void removeExpiredEvents();
	void handleNewCommands();
	void handleNewEvents();
};