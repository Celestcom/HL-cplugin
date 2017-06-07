#pragma once
#include <boost/variant.hpp>
#include "ParameterizedEvent.h"

struct CurveEvent : public ParameterizedEvent {
	float Time;
	std::vector<float> Volumes;
	std::vector<float> TimePoints;
	CurveEvent();
	bool doSetFloat(const char* key, float value) override;
	bool doSetFloats(const char* key, float* values, unsigned int length) override;
};


