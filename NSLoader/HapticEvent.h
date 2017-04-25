#pragma once
#include "Enums.h"
#include <boost\uuid\uuid.hpp>
#include <string>
class HapticEvent
{
public:
	HapticEvent(std::string effect, float duration, boost::uuids::uuid handle, float strength);
	HapticEvent() {}
	~HapticEvent();
	boost::uuids::uuid Handle;
	bool Dirty;
	std::string Effect;
	float Strength;
	float Duration;
	float TimeElapsed;
	bool Sent;
	::Duration DurationType() const;
	bool operator==(const HapticEvent& rhs);
};

