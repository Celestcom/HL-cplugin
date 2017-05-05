#pragma once

#include "IHapticDevice.h"
#include "HardwareDriver.h"
#include "IRetainedEvent.h"
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
	void update(float dt);
private:
	void createRetained(boost::uuids::uuid handle, const SuitEvent& event) override;
	void controlRetained(boost::uuids::uuid handle, NSVR_PlaybackCommand command) override;
};




class HardlightDevice : public IHapticDevice {
public:
	HardlightDevice();

	virtual void RegisterDrivers(EventRegistry& registry) override;


	virtual void UnregisterDrivers(EventRegistry& registry) override;

private:
	std::shared_ptr<Hardlight_Mk3_ZoneDriver> chestLeft;
};

