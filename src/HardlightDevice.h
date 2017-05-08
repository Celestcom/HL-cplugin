#pragma once

#include "IHapticDevice.h"
#include "HardwareDriver.h"
#include "IRetainedEvent.h"
#include "EffectCommand.pb.h"
#include <queue>
#include <boost/uuid/uuid.hpp>


typedef std::vector<NullSpaceIPC::EffectCommand> CommandBuffer;

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

	virtual boost::uuids::uuid Id() const override;
	Hardlight_Mk3_ZoneDriver();
private:
	void createRetained(boost::uuids::uuid handle, const SuitEvent& event) override;
	void controlRetained(boost::uuids::uuid handle, NSVR_PlaybackCommand command) override;
	boost::uuids::uuid m_id;
};




class HardlightDevice : public IHapticDevice {
public:
	HardlightDevice();

	virtual void RegisterDrivers(EventRegistry& registry) override;


	virtual void UnregisterDrivers(EventRegistry& registry) override;

private:
	std::shared_ptr<Hardlight_Mk3_ZoneDriver> chestLeft;
};




