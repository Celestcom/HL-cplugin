#pragma once
#include <boost/variant.hpp>
#include "ParameterizedEvent.h"

struct BasicHapticEvent : public ParameterizedEvent {
	float Time;
	float Strength;
	float Duration;
	uint32_t Area;
	std::string ParsedEffectFamily;
	uint32_t RequestedEffectFamily;
	BasicHapticEvent(float time, float strength, float duration, uint32_t area, std::string effect);
	BasicHapticEvent();
	bool doSetFloat(const char* key, float value) override;
	bool doSetInt(const char* key, int value) override;

};


typedef boost::variant<BasicHapticEvent> SuitEvent;
