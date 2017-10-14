#include "stdafx.h"
#include "PlayableEvent.h"
#include "ParameterizedEvent.h"
#include "HLVR.h"
#include "BasicHapticEvent.h"
#include <typeinfo>
#include "Locator.h"
#include <bitset>
#include "SharedTypes.h"




Validator make_xor_constraint(Validator lhs, Validator rhs) {
	return [lhs, rhs](const ParameterizedEvent& event, std::vector<HLVR_Event_KeyParseResult>* results) {
		return lhs(event, results) ^ rhs(event, results);
	};
}

Validator make_or_constraint(Validator lhs, Validator rhs) {
	return [lhs, rhs](const ParameterizedEvent& event, std::vector<HLVR_Event_KeyParseResult>* results) {
		return lhs(event, results) || rhs(event, results);
	};
}

Validator make_and_constraint(Validator lhs, Validator rhs) {
	return [lhs, rhs](const ParameterizedEvent& event, std::vector<HLVR_Event_KeyParseResult>* results) {
		return lhs(event, results) && rhs(event, results);
	};
}




bool PlayableEvent::operator<(const PlayableEvent & rhs) const
{
	return this->time() < rhs.time();
}

std::unique_ptr<PlayableEvent>
PlayableEvent::make(HLVR_EventType type, float timeOffset)
{
	std::unique_ptr<PlayableEvent> possibleEvent;
	switch (type) {
	case HLVR_EventType::HLVR_EventType_SimpleHaptic:
		possibleEvent = std::make_unique<BasicHapticEvent>(timeOffset);
		break;
	default:
		break;
	}

	return possibleEvent;


}

bool PlayableEvent::operator==(const PlayableEvent& other) const
{
	return typeid(*this) == typeid(other) && m_time == other.m_time &&  isEqual(other);
}








bool cmp_by_time(const std::unique_ptr<PlayableEvent>& lhs, const std::unique_ptr<PlayableEvent>& rhs)
{
	return lhs->time() < rhs->time();
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



bool cmp_by_duplicate(const std::unique_ptr<PlayableEvent>& lhs, const std::unique_ptr<PlayableEvent>& rhs) {
	return *lhs == *rhs;
}

void PlayableEvent::debug_parse(const ParameterizedEvent & event, HLVR_Event_ValidationResult * result) const
{
	*result = { 0 };

	std::vector<HLVR_Event_KeyParseResult> results;
	std::vector<Validator> validators = make_validators();

	for (auto& validator : validators) {
		validator(event, &results);
	}

	std::copy(results.begin(), results.end(), result->Errors);

	result->Count = results.size();
}
