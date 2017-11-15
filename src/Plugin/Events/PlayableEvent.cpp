#include "stdafx.h"
#include "PlayableEvent.h"
#include "ParameterizedEvent.h"
#include "HLVR.h"
#include <typeinfo>
#include "Locator.h"
#include <bitset>
#include "SharedTypes.h"
#include "AnalogAudio.h"
#include "DiscreteHapticEvent.h"
#include "ContinuousHaptic.h"
#include "BufferedHaptic.h"
#pragma warning(push)
#pragma warning(disable : 4267)
#include "HighLevelEvent.pb.h"
#pragma warning(pop)





class serialize_target_visitor : public boost::static_visitor<void> {

public:
	serialize_target_visitor(NullSpaceIPC::Location* location) : m_location(location) {

	}
	void operator()(const TargetRegions& regions) {
		auto mut_regions = m_location->mutable_regions();
		for (auto region : regions.regions) {
			mut_regions->add_regions(region);
		}
	}
	void operator()(const TargetNodes& nodes) {
		auto mut_nodes = m_location->mutable_nodes();
		for (auto node : nodes.nodes) {
			mut_nodes->add_nodes(node);
		}
	}
	
private:
	NullSpaceIPC::Location* m_location;
};



void PlayableEvent::parse(const ParameterizedEvent & e)
{
	TargetRegions regions;
	TargetNodes nodes;


	if (e.TryGet(HLVR_EventKey_Target_Regions_UInt32s, &regions.regions)) {
		m_target = regions;
	}
	else if (e.TryGet(HLVR_EventKey_Target_Nodes_UInt32s, &nodes.nodes)) {
		m_target = nodes;
	}
	else {
		m_target = TargetRegions{ {hlvr_region_body} };
	}

	doParse(e); 
}

bool PlayableEvent::operator<(const PlayableEvent & rhs) const
{
	return this->time() < rhs.time();
}

void PlayableEvent::serialize(NullSpaceIPC::HighLevelEvent & event) const
{
	NullSpaceIPC::LocationalEvent* location = event.mutable_locational_event();

	serialize_target_visitor extractor(location->mutable_location());
	boost::apply_visitor(extractor, m_target);
	
	doSerialize(event);
}

std::unique_ptr<PlayableEvent>
PlayableEvent::make(HLVR_EventType type, float timeOffset)
{
	switch (type) {
	case HLVR_EventType::HLVR_EventType_DiscreteHaptic:
		return std::make_unique<DiscreteHapticEvent>(timeOffset);
	case HLVR_EventType::HLVR_EventType_BeginAnalogAudio:
		return std::make_unique<BeginAnalogAudio>(timeOffset);
	case HLVR_EventType::HLVR_EventType_EndAnalogAudio:
		return std::make_unique<EndAnalogAudio>(timeOffset);
	case HLVR_EventType::HLVR_EventType_BufferedHaptic:
		return std::make_unique<BufferedHaptic>(timeOffset);
	default:
		break;
	}

	return std::unique_ptr<PlayableEvent>();


}

bool PlayableEvent::operator==(const PlayableEvent& other) const
{
	return 
		typeid(*this) == typeid(other) 
	 && m_time == other.m_time 
	 && m_target == other.m_target
	 && isEqual(other);
}









std::unordered_map<uint32_t, NullSpace::SharedMemory::nsvr_shared_region::_enumerated> regionMap =
{
	{ 0, NullSpace::SharedMemory::nsvr_shared_region::identifier_lower_arm_left },
	{ 1,  NullSpace::SharedMemory::nsvr_shared_region::identifier_upper_arm_left },
	{ 2, NullSpace::SharedMemory::nsvr_shared_region::identifier_shoulder_left },
	{ 3, NullSpace::SharedMemory::nsvr_shared_region::identifier_upper_back_left},
	{ 4, NullSpace::SharedMemory::nsvr_shared_region::identifier_chest_left },
	{ 5, NullSpace::SharedMemory::nsvr_shared_region::identifier_upper_ab_left },
	{ 6, NullSpace::SharedMemory::nsvr_shared_region::identifier_middle_ab_left },
	{ 7, NullSpace::SharedMemory::nsvr_shared_region::identifier_lower_ab_left },
	{ 16, NullSpace::SharedMemory::nsvr_shared_region::identifier_lower_arm_right },
	{ 17, NullSpace::SharedMemory::nsvr_shared_region::identifier_upper_arm_right},
	{ 18, NullSpace::SharedMemory::nsvr_shared_region::identifier_shoulder_right },
	{ 19, NullSpace::SharedMemory::nsvr_shared_region::identifier_upper_back_right },
	{ 20, NullSpace::SharedMemory::nsvr_shared_region::identifier_chest_right },
	{ 21, NullSpace::SharedMemory::nsvr_shared_region::identifier_upper_ab_right },
	{ 22, NullSpace::SharedMemory::nsvr_shared_region::identifier_middle_ab_right },
	{ 23, NullSpace::SharedMemory::nsvr_shared_region::identifier_lower_ab_right }
};




PlayableEvent::PlayableEvent(float time) : m_time(time) 
{
}

float PlayableEvent::time() const
{
	return m_time; 
}

void PlayableEvent::debug_parse(const ParameterizedEvent & event, HLVR_Event_ValidationResult * result) const
{
	*result = { 0 };
	//todo: make some validators for target?

	std::vector<HLVR_Event_KeyParseResult> results;
	std::vector<Validator> validators = makeValidators();

	for (auto& validator : validators) {
		validator(event, &results);
	}

	std::copy(results.begin(), results.end(), result->Errors);

	result->Count = results.size();
}
