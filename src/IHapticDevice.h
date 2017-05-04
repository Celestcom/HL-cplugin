#pragma once
#include <vector>
#include "SuitEvent.h"
#include "NSLoader_fwds.h"

struct ImmediateArgs {
	uint32_t location;
	float strength;
};
class IHapticDevice {
public:
	virtual void CreateRetained(uint32_t handle, std::vector<SuitEvent> events) {}
	virtual void ControlRetained(uint32_t handle, NSVR_PlaybackCommand command) {}
	virtual void Immediate(const std::vector<ImmediateArgs>& args) {}
};