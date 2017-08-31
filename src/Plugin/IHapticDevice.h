#pragma once
#include "EventRegistry.h"
#include "EffectCommand.pb.h"

#include "AreaFlags.h"
typedef std::vector<NullSpaceIPC::EffectCommand> CommandBuffer;

struct HapticDisplayInfo {
public:
	uint16_t strength;
	uint32_t family;
	AreaFlag area;
};

typedef std::vector<HapticDisplayInfo> DisplayResults;

class IHapticDevice {
public:
	virtual void RegisterDrivers(EventRegistry& registry) = 0;
	virtual void UnregisterDrivers(EventRegistry& registry) = 0;

	virtual CommandBuffer GenerateHardwareCommands(float dt) = 0;

	virtual DisplayResults QueryDrivers() = 0;
};