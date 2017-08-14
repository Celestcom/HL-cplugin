#include "stdafx.h"
#include "BasicHapticEvent.h"
#include "Locator.h"
#pragma warning(push)
#pragma warning(disable : 4267)
#include "HighLevelEvent.pb.h"
#pragma warning(pop)

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
	m_area = ev.Get<int>("area", (int)AreaFlag::None);
	m_requestedEffectFamily = ev.Get<int>("effect", 1);
	std::string effect = Locator::getTranslator().ToEffectFamilyString(m_requestedEffectFamily);
	m_parsedEffectFamily = effect;
	
	return true;

}

bool BasicHapticEvent::isEqual(const PlayableEvent& other) const
{
	const auto& ev = static_cast<const BasicHapticEvent&>(other);
	return 
		   m_time == ev.m_time
		&& m_strength == ev.m_strength
		&& m_requestedEffectFamily == ev.m_requestedEffectFamily
		&& m_parsedEffectFamily == ev.m_parsedEffectFamily
		&& m_area == ev.m_area
		&& m_duration == ev.m_duration;
}

void BasicHapticEvent::serialize(NullSpaceIPC::HighLevelEvent& event) const
{
	using namespace NullSpaceIPC;
	SimpleHaptic* simple = event.mutable_simple_haptic();
	simple->set_duration(m_duration);
	simple->set_effect(m_requestedEffectFamily);
	simple->set_strength(m_strength);

	auto regions = extractRegions(*this);
	for (const auto& region : regions) {
		simple->add_regions(region);
	}
	

}

float BasicHapticEvent::strength() const
{
	return m_strength;
}



NSVR_EventType BasicHapticEvent::type() const
{
	return descriptor;
}
