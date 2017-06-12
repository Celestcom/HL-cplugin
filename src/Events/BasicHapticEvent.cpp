#include "stdafx.h"
#include "BasicHapticEvent.h"
#include "Locator.h"


BasicHapticEvent::BasicHapticEvent() : 
	PlayableEvent(),
	m_time(0),
	m_strength(1),
	m_duration(0),
	m_area(0)
{
	m_parsedEffectFamily = "click";
	m_requestedEffectFamily = Locator::getTranslator().ToEffectFamily(m_parsedEffectFamily);
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

bool BasicHapticEvent::parse(const ParameterizedEvent& ev)
{

	m_time = ev.Get<float>("time", 0.0f);
	m_strength = ev.Get<float>("strength", 1.0f);
	m_duration = ev.Get<float>("duration", 0.0f);
	m_area = ev.Get<int>("area", 0);
	m_requestedEffectFamily = ev.Get<int>("effect", 1);
	std::string effect = Locator::getTranslator().ToEffectFamilyString(m_requestedEffectFamily);
	m_parsedEffectFamily = effect;
	
	return true;

}

float BasicHapticEvent::strength() const
{
	return m_strength;
}



NSVR_EventType BasicHapticEvent::type() const
{
	return descriptor;
}
