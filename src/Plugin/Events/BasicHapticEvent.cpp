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
	using e = BasicHapticEvent;
	where_visitor(NullSpaceIPC::Location* location) : m_location(location) {

	}
	void operator()(const std::vector<e::Loc<e::region>>& regions) {
		
		auto mut_regions = m_location->mutable_regions();
		for (auto region : regions) {
			mut_regions->add_regions(region.value);
		}
	}
	void operator()(const std::vector<e::Loc<e::node>>& nodes) {
		auto mut_nodes = m_location->mutable_nodes();
		for (auto node : nodes) {
			mut_nodes->add_nodes(node.value);
		}
	}
private:
	NullSpaceIPC::Location* m_location;
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

template<typename Phantom, typename Original>
std::vector<Phantom> wrap_type(const std::vector<Original>& original) {
	std::vector<Phantom> desired;
	desired.reserve(original.size());
	for (const auto& item : original) {
		desired.push_back(Phantom{item });
	}
	return desired;
}
bool BasicHapticEvent::parse(const ParameterizedEvent& ev)
{

	m_time = ev.GetOr<float>(HLVR_EventKey_Time_Float, 0.0f);
	m_strength = ev.GetOr<float>(HLVR_EventKey_SimpleHaptic_Strength_Float, 1.0f);
	m_duration = ev.GetOr<float>(HLVR_EventKey_SimpleHaptic_Duration_Float, 0.0f);

	std::vector<uint32_t> regions;
	std::vector<uint32_t> nodes;
	if (ev.TryGet(HLVR_EventKey_SimpleHaptic_Where_Regions_UInt32s, &regions)) {
		m_area = wrap_type<Loc<region>>(regions);
	}
	else if (ev.TryGet(HLVR_EventKey_SimpleHaptic_Where_Nodes_UInt32s, &nodes)) {
		m_area = wrap_type<Loc<node>>(nodes);
	}
	else {
		//possibly should fail here. Need to make clear where failures happen with event parsing.
		m_area = wrap_type<Loc<region>>(std::vector<uint32_t>{hlvr_region_UNKNOWN});
	}


	m_requestedEffectFamily = ev.GetOr<int>(HLVR_EventKey_SimpleHaptic_Effect_Int, 3);
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
	LocationalEvent* location = event.mutable_locational_event();
	SimpleHaptic* simple = location->mutable_simple_haptic();
	simple->set_duration(m_duration);
	simple->set_effect(m_requestedEffectFamily);
	simple->set_strength(m_strength);

	where_visitor where(location->mutable_location());
	boost::apply_visitor(where, m_area);
	
}

float BasicHapticEvent::strength() const
{
	return m_strength;
}



HLVR_EventType BasicHapticEvent::type() const
{
	return descriptor;
}
