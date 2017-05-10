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



class Hardlight_Mk3_ZoneDriver : public HardwareDriver {
public:
	std::vector<GeneratedEvent> m_events;
	CommandBuffer update(float dt);

	virtual boost::uuids::uuid Id() const override;
	//this should really take a Location probably..
	Hardlight_Mk3_ZoneDriver(Location area);
private:
	void createRetained(boost::uuids::uuid handle, const SuitEvent& event) override;
	void controlRetained(boost::uuids::uuid handle, NSVR_PlaybackCommand command) override;
	boost::uuids::uuid m_id;
	uint32_t m_area;
};




class HardlightDevice : public IHapticDevice {
public:
	HardlightDevice();

	virtual void RegisterDrivers(EventRegistry& registry) override;


	virtual void UnregisterDrivers(EventRegistry& registry) override;


	virtual CommandBuffer GenerateHardwareCommands(float dt) override;

private:
	std::shared_ptr<Hardlight_Mk3_ZoneDriver> chestLeft;
};




