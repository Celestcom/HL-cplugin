#include "stdafx.h"
#include "CurveEvent.h"


CurveEvent::CurveEvent() : 
	PlayableEvent(),
	m_area(0),
	m_duration(0),
	m_time(0),
	m_timePoints(),
	m_volumes()
{
	
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
	return m_duration;
}
