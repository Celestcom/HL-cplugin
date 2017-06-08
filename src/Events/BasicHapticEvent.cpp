#include "stdafx.h"
#include "BasicHapticEvent.h"
#include "Locator.h"


BasicHapticEvent::BasicHapticEvent() : 
	ParameterizedEvent(), 
	PlayableEvent(),
	m_time(0),
	m_strength(1),
	m_duration(0),
	m_area(0)
{
	m_parsedEffectFamily = "click";
	m_requestedEffectFamily = Locator::getTranslator().ToEffectFamily(m_parsedEffectFamily);
}


bool BasicHapticEvent::doSetFloat(const char * key, float value)
{
	if (strcmp("strength", key) == 0) {
		m_strength = minimum_bound(0.0f, value);
		return true;
	}
	else if (strcmp("time", key) == 0) {
		m_time = minimum_bound(0.0f, value);
		return true;
	}
	else if (strcmp("duration", key) == 0) {
		m_duration = minimum_bound(0.0f, value);
		return true;
	}
	
	return false;
}

bool BasicHapticEvent::doSetInt(const char * key, int value)
{
	if (strcmp("area", key) == 0) {
		m_area = minimum_bound(0, value);
		return true;
	}
	else if (strcmp("effect", key) == 0) {
		value = minimum_bound(1, value); //1 is bump
		std::string effect = Locator::getTranslator().ToEffectFamilyString(value);
		m_parsedEffectFamily = effect;
		m_requestedEffectFamily = value;
		return true;
	}

	return false;
}




uint32_t BasicHapticEvent::area() const
{
	return m_area;
}

float BasicHapticEvent::time() const
{
	return m_time;
}

float BasicHapticEvent::duration() const
{
	//Oneshots, aka 0 duration effects, last about a quarter second
	return m_duration == 0 ? 0.25f : m_duration;
}

uint32_t BasicHapticEvent::effectFamily() const
{
	return m_requestedEffectFamily;
}

float BasicHapticEvent::strength() const
{
	return m_strength;
}

BasicHapticEvent * BasicHapticEvent::doClone()
{
	return new BasicHapticEvent(*this);
}

NSVR_EventType BasicHapticEvent::type() const
{
	return descriptor;
}
