#include "stdafx.h"
#include "PlayableEvent.h"
#include "ParameterizedEvent.h"
#include "NSLoader.h"
#include "BasicHapticEvent.h"
#include "CurveEvent.h"
#include <typeinfo>
#include "Locator.h"
#include <bitset>
bool PlayableEvent::operator<(const PlayableEvent & rhs) const
{
	return this->time() < rhs.time();
}

std::unique_ptr<PlayableEvent>
PlayableEvent::make(NSVR_EventType type)
{
	std::unique_ptr<PlayableEvent> possibleEvent;
	switch (type) {
	case NSVR_EventType::NSVR_EventType_BasicHapticEvent:
		possibleEvent = std::make_unique<BasicHapticEvent>();
		break;
	case NSVR_EventType::NSVR_EventType_CurveHapticEvent:
		possibleEvent = std::make_unique<CurveEvent>();
		break;
	default:
		break;
	}

	return possibleEvent;


}

bool PlayableEvent::operator==(const PlayableEvent& other) const
{
	return typeid(*this) == typeid(other) && isEqual(other);
}








bool cmp_by_time(const std::unique_ptr<PlayableEvent>& lhs, const std::unique_ptr<PlayableEvent>& rhs)
{
	return lhs->time() < rhs->time();
}



std::array<std::string, 32> regionMap = {

	"left_forearm",
	"left_upper_arm",
	"left_shoulder",
	"left_back",
	"left_upper_chest",
	"left_upper_ab",
	"left_mid_ab",
	"left_lower_ab",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"right_forearm",
	"right_upper_arm",
	"right_shoulder",
	"right_back",
	"right_upper_chest",
	"right_upper_ab",
	"right_mid_ab",
	"right_lower_ab",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved"
};

std::vector<std::string> extractRegions(const PlayableEvent & event)
{
	auto& translator = Locator::getTranslator();
	std::vector<std::string> regions;
	std::bitset<32> areas(event.area());
	regions.reserve(areas.count());

	for (std::size_t i = 0; i < areas.size(); i++) {
		if (areas.test(i)) {
			regions.push_back(regionMap.at(i));
		}
	}
	return regions;
}

bool cmp_by_duplicate(const std::unique_ptr<PlayableEvent>& lhs, const std::unique_ptr<PlayableEvent>& rhs) {
	return *lhs == *rhs;
}

