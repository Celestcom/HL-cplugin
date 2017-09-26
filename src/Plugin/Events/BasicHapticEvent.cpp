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
	m_area()
{
	m_parsedEffectFamily = "click";
	m_requestedEffectFamily = Locator::getTranslator().ToEffectFamily(m_parsedEffectFamily);
}



class where_visitor : public boost::static_visitor<void> {

public:
	where_visitor(NullSpaceIPC::SimpleHaptic* haptic) : m_haptic(haptic) {

	}
	void operator()(const std::vector<uint32_t>& regions) {
		auto mut_regions = m_haptic->mutable_regions();
		for (uint32_t region : regions) {
			mut_regions->add_regions(region);
		}
	}
	void operator()(const std::vector<uint64_t>& nodes) {
		auto mut_nodes = m_haptic->mutable_nodes();
		for (uint64_t node : nodes) {
			mut_nodes->add_nodes(node);
		}
	}
private:
	NullSpaceIPC::SimpleHaptic* m_haptic;
};

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

	m_time = ev.GetOr<float>(NSVR_EventKey_Time_Float, 0.0f);
	m_strength = ev.GetOr<float>(NSVR_EventKey_SimpleHaptic_Strength_Float, 1.0f);
	m_duration = ev.GetOr<float>(NSVR_EventKey_SimpleHaptic_Duration_Float, 0.0f);

	std::vector<uint32_t> regions;
	std::vector<uint64_t> nodes;
	if (ev.TryGet(NSVR_EventKey_SimpleHaptic_Regions_UInt32s, &regions)) {
		m_area = regions;
	}
	else if (ev.TryGet(NSVR_EventKey_SimpleHaptic_Nodes_UInt64s, &nodes)) {
		m_area = nodes;
	}
	else {
		//possibly should fail here. Need to make clear where failures happen with event parsing.
		m_area = std::vector<uint32_t>{ nsvr_region_unknown };
	}


	m_requestedEffectFamily = ev.GetOr<int>(NSVR_EventKey_SimpleHaptic_Effect_Int, 1);
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

	where_visitor where(simple);
	boost::apply_visitor(where, m_area);
	
}

float BasicHapticEvent::strength() const
{
	return m_strength;
}



NSVR_EventType BasicHapticEvent::type() const
{
	return descriptor;
}
