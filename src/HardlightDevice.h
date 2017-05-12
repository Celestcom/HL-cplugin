#pragma once

#include "IHapticDevice.h"
#include "HardwareDriver.h"
#include "IRetainedEvent.h"
#include "EffectCommand.pb.h"
#include <queue>
#include <boost/uuid/uuid.hpp>
#include "Enums.h"


class GeneratedEvent {
public:
	boost::uuids::uuid id;
	std::unique_ptr<IRetainedEvent> event;
	GeneratedEvent(boost::uuids::uuid id, std::unique_ptr<IRetainedEvent> event);
	bool operator==(const GeneratedEvent& other);
};


class ZoneModel {
public:
	//todo: may need a mutex to protect access to command buffer?
	//nullptr signals nothing playing
	//if something is playing, returns a non-owning pointer to it
	IRetainedEvent* CurrentlyPlaying();
	void Put(boost::uuids::uuid id, std::unique_ptr<IRetainedEvent> event);
	void Remove(boost::uuids::uuid id);
	void Play(boost::uuids::uuid id);
	void Pause(boost::uuids::uuid id);
	CommandBuffer Update(float dt);
private:
	CommandBuffer m_commands;
	std::vector<GeneratedEvent> m_events;


};

class RtpModel {
public:
	
	RtpModel();
	void ChangeVolume(int newVolume);
	CommandBuffer Update(float dt);
private:
	int m_volume;
	CommandBuffer m_commands;
};
class Hardlight_Mk3_ZoneDriver : public HardwareDriver {
public:
	CommandBuffer update(float dt);

	virtual boost::uuids::uuid Id() const override;
	//this should really take a Location probably..
	Hardlight_Mk3_ZoneDriver(Location area);

	Location Location();


private:
	ZoneModel m_retainedModel;
	RtpModel m_rtpModel;
	void createRetained(boost::uuids::uuid handle, const SuitEvent& event) override;
	void controlRetained(boost::uuids::uuid handle, NSVR_PlaybackCommand command) override;
	void realtime(const RealtimeArgs& args) override;

	enum class Mode {Retained, Realtime};
	void transition(Mode mode);
	Mode m_currentMode;
	boost::uuids::uuid m_id;
	uint32_t m_area;

	CommandBuffer& m_commands;
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




