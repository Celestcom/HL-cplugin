#pragma once
#include <vector>
#include "SuitEvent.h"
#include "IHapticDevice.h"
#include "HapticsPlayer.h"
#include "HapticDriver.h"
#include "Enums.h"


class HardlightDevice : public IHapticDevice {
public:
	virtual void CreateRetained(uint32_t handle, std::vector<SuitEvent> events) override;
	virtual void ControlRetained(uint32_t handle, NSVR_PlaybackCommand command) override;
	//virtual void Immediate(const std::vector<ImmediateArgs>& args) {}
private:
	HapticsPlayer m_player;
	std::unordered_map<Location, DriverSet> m_drivers;
};