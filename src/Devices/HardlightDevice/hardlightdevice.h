#pragma once

#include "IHapticDevice.h"
#include "HardlightMk3ZoneDriver.h"


class HardlightDevice : public IHapticDevice {
public:
	HardlightDevice();

	virtual void RegisterDrivers(EventRegistry& registry) override;


	virtual void UnregisterDrivers(EventRegistry& registry) override;


	virtual CommandBuffer GenerateHardwareCommands(float dt) override;




	virtual DisplayResults QueryDrivers() override;

private:
	std::vector<std::shared_ptr<Hardlight_Mk3_ZoneDriver>> m_drivers;


};




