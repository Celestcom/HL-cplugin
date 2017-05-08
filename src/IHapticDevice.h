#pragma once
#include "EventRegistry.h"
#include "EffectCommand.pb.h"
typedef std::vector<NullSpaceIPC::EffectCommand> CommandBuffer;


class IHapticDevice {
public:
	virtual void RegisterDrivers(EventRegistry& registry) = 0;
	virtual void UnregisterDrivers(EventRegistry& registry) = 0;

	virtual CommandBuffer GenerateHardwareCommands(float dt) = 0;
};