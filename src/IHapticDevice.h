#pragma once
#include "EventRegistry.h"


class IHapticDevice {
public:
	virtual void RegisterDrivers(EventRegistry& registry) = 0;
	virtual void UnregisterDrivers(EventRegistry& registry) = 0;
};