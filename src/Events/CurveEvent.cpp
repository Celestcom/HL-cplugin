#include "stdafx.h"
#include "CurveEvent.h"


CurveEvent::CurveEvent() : 
	ParameterizedEvent(), 
	PlayableEvent(),
	m_area(0),
	m_duration(0),
	m_time(0),
	m_timePoints(),
	m_volumes()
{
	
}



bool CurveEvent::doSetFloat(const char * key, float value)
{
	if (strcmp("time", key) == 0) {
		m_time = minimum_bound(0.0f, value);
		return true;
	}
	return false;
}

bool CurveEvent::doSetFloats(const char* key, float* values, unsigned int length)
{

	//note: not constraining to be positive at this point
	if (strcmp("volumes", key) == 0) {
		m_volumes.reserve(length);
		memcpy(&m_volumes[0], &values[0], length);
		m_volumes.resize(length);
		return true;
	}

	if (strcmp("time-points", key) == 0) {
		m_timePoints.reserve(length);
		memcpy(&m_timePoints[0], &values[0], length);
		m_timePoints.resize(length);
		return true;
	}
	return false;
}

bool CurveEvent::doSetInt(const char * key, int value)
{
	if (strcmp("area", key) == 0) {
		m_area = minimum_bound(0, value);
		return true;
	}

	return false;
}

CurveEvent * CurveEvent::doClone()
{
	return new CurveEvent(*this);
}

float CurveEvent::time() const
{
	return m_time;
}

uint32_t CurveEvent::area() const
{
	return m_area;
}

NSVR_EventType CurveEvent::type() const
{
	return descriptor;
}

float CurveEvent::duration() const
{
	m_duration;
}
