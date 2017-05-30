#pragma once

#include "IHapticDevice.h"
#include "HardwareDriver.h"
#include "IRetainedEvent.h"
#include "EffectCommand.pb.h"
#include <queue>
#include <deque>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include "Enums.h"
#include <functional>
#include <mutex>
#include <boost/optional.hpp>
#include <boost/lockfree/spsc_queue.hpp>


class BasicHapticEventData {
public:
	uint32_t effect;
	uint32_t area;
	float duration;
	float strength;
};

//Todo: This is not good design. The plugin shouldn't have to know the specific firmware commands.
//We should push this knowledge into the Driver so that the plugin can issue PLAY_DURATION and it will issue
//HALT - PLAY_CONT - or whatever is actually necessary to make the hardware respond correctly.
class Hardlight_Mk3_Firmware {
public:
	static NullSpaceIPC::EffectCommand generateContinuousPlay(const BasicHapticEventData& data);
	static NullSpaceIPC::EffectCommand generateOneshotPlay(const BasicHapticEventData& data);
	static NullSpaceIPC::EffectCommand generateHalt(uint32_t area);
};
class LiveBasicHapticEvent {
public:
	LiveBasicHapticEvent();
	LiveBasicHapticEvent(boost::uuids::uuid parentId, boost::uuids::uuid uniqueId, BasicHapticEventData data);
	const BasicHapticEventData& Data() const;
	bool operator==(const LiveBasicHapticEvent& other) const;
	void update(float dt);
	bool isFinished() const;
	bool isContinuous() const;
	bool isOneshot() const;
	bool isChildOf(const boost::uuids::uuid& parentId) const;
	boost::uuids::uuid GetParentId();
private:
	boost::uuids::uuid parentId;
	boost::uuids::uuid uniqueId;
	float currentTime;
	bool isPlaying;
	BasicHapticEventData eventData;
};

class MotorStateChanger {
public:
	MotorStateChanger(uint32_t areaId);
	enum class MotorFirmwareState {Idle, PlayingOneshot, PlayingContinuous};
	MotorFirmwareState GetState() const;
	CommandBuffer transitionTo(const LiveBasicHapticEvent& event);
	CommandBuffer transitionToIdle();
private:
	MotorFirmwareState currentState;
	boost::optional<LiveBasicHapticEvent> previousContinuous;
	uint32_t area;
	CommandBuffer transitionToOneshot(BasicHapticEventData data);
	CommandBuffer transitionToContinuous(BasicHapticEventData data);
};




class ZoneModel {
public:

	ZoneModel(uint32_t area);

	void Put(LiveBasicHapticEvent event);
	void Remove(boost::uuids::uuid id);
	void Play(boost::uuids::uuid id);
	void Pause(boost::uuids::uuid id);


	typedef std::vector<LiveBasicHapticEvent> PlayingContainer;
	typedef std::vector<LiveBasicHapticEvent> PausedContainer;

	const PausedContainer& PausedEvents();
	const PlayingContainer& PlayingEvents();

	CommandBuffer Update(float dt);

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


class RtpModel {
public:
	
	RtpModel(uint32_t area);
	void ChangeVolume(int newVolume);
	CommandBuffer Update(float dt);
private:
	int m_volume;
	uint32_t m_area;
	CommandBuffer m_commands;
	std::mutex m_mutex;
};
class Hardlight_Mk3_ZoneDriver : public HardwareDriver {
public:
	CommandBuffer update(float dt);

	virtual boost::uuids::uuid Id() const override;
	//this should really take a Location probably..
	Hardlight_Mk3_ZoneDriver(Location area);

	Location Location();


private:
	uint32_t m_area;

	ZoneModel m_retainedModel;
	RtpModel m_rtpModel;
	void createRetained(boost::uuids::uuid handle, const SuitEvent& event) override;
	void controlRetained(boost::uuids::uuid handle, NSVR_PlaybackCommand command) override;
	void realtime(const RealtimeArgs& args) override;

	boost::uuids::random_generator m_gen;
	enum class Mode {Retained, Realtime};
	void transitionInto(Mode mode);
	Mode m_currentMode;
	boost::uuids::uuid m_parentId;
	std::mutex m_mutex;
	CommandBuffer m_commands;
};





class HardlightDevice : public IHapticDevice {
public:
	HardlightDevice();

	virtual void RegisterDrivers(EventRegistry& registry) override;


	virtual void UnregisterDrivers(EventRegistry& registry) override;


	virtual CommandBuffer GenerateHardwareCommands(float dt) override;

private:
	std::vector<std::shared_ptr<Hardlight_Mk3_ZoneDriver>> m_drivers;


};




