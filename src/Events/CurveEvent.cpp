#include "stdafx.h"
#include "CurveEvent.h"


CurveEvent::CurveEvent() : ParameterizedEvent()
{
	
}



bool CurveEvent::doSetFloat(const char * key, float value)
{
	if (strcmp("time", key) == 0) {
		Time = minimum_bound(0.0f, value);
		return true;
	}
	return false;
}

bool CurveEvent::doSetFloats(const char* key, float* values, unsigned int length)
{

	//note: not constraining to be positive at this point
	if (strcmp("volumes", key) == 0) {
		Volumes.reserve(length);
		memcpy(&Volumes[0], &values[0], length);
		Volumes.resize(length);
		return true;
	}

	if (strcmp("time-points", key) == 0) {
		TimePoints.reserve(length);
		memcpy(&TimePoints[0], &values[0], length);
		TimePoints.resize(length);
		return true;
	}
	return false;
}
