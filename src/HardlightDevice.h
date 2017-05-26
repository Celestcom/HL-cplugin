#pragma once

#include "IHapticDevice.h"
#include "HardwareDriver.h"
#include "IRetainedEvent.h"
#include "EffectCommand.pb.h"
#include <queue>
#include <deque>
#include <boost/uuid/uuid.hpp>
#include "Enums.h"
#include <functional>
#include <mutex>


class MyBasicHapticEvent {
public:
	MyBasicHapticEvent(boost::uuids::uuid id, uint32_t area, float duration, float strength, uint32_t effect);

	explicit MyBasicHapticEvent(boost::uuids::uuid id);

	bool MyBasicHapticEvent::operator==(const MyBasicHapticEvent & other);
	bool IsFunctionallyIdentical(const MyBasicHapticEvent &other);

	void EmitCreationCommands(CommandBuffer* buffer) const;
	void EmitCleanupCommands(CommandBuffer* buffer) const;



	void Update(float dt);

	void LogicalPlay();
	void LogicalPause();

	bool Finished() const;
	boost::uuids::uuid m_id;
	uint32_t m_area;
	float m_duration;
	float m_strength;
	uint32_t m_effect;
	float m_time;

	bool m_playing;
	


	

};


class ZoneModel {
public:
	typedef std::deque<MyBasicHapticEvent> PlayingContainer;
	typedef std::vector<MyBasicHapticEvent> PausedContainer;
	//todo: may need a mutex to protect access to command buffer?
	//nullptr signals nothing playing
	//if something is playing, returns a non-owning pointer to it
	void Put(MyBasicHapticEvent event);
	ZoneModel::PlayingContainer::iterator Remove(boost::uuids::uuid id);
	void Play(boost::uuids::uuid id);
	const PausedContainer& PausedEvents();
	const PlayingContainer& PlayingEvents();
	ZoneModel::PlayingContainer::iterator  Pause(boost::uuids::uuid id);
	CommandBuffer Update(float dt);

	ZoneModel();
private:
	
	inline PlayingContainer::iterator findPlayingEvent(const boost::uuids::uuid& id);
	inline PausedContainer::iterator findPausedEvent(const boost::uuids::uuid& id);

	void setCreationCommands(std::function<void(CommandBuffer* buffer)>);
	void setCleanupCommands(std::function<void(CommandBuffer* buffer)>);
	CommandBuffer m_creationCommands;
	CommandBuffer m_cleanupCommands;
	PlayingContainer m_events;
	PausedContainer m_pausedEvents;
	std::mutex m_mutex;


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

	enum class Mode {Retained, Realtime};
	void transitionInto(Mode mode);
	Mode m_currentMode;
	boost::uuids::uuid m_id;
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




