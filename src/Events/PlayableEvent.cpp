#include "stdafx.h"
#include "PlayableEvent.h"
#include "ParameterizedEvent.h"
#include "NSLoader.h"
#include "BasicHapticEvent.h"
#include "CurveEvent.h"
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
		possibleEvent = std::unique_ptr<PlayableEvent>(new BasicHapticEvent());
		break;
	case NSVR_EventType::NSVR_EventType_CurveHapticEvent:
//		possibleEvent = std::unique_ptr<PlayableEvent>(new CurveEvent());
		break;
	default:
		break;
	}

	return possibleEvent;


}

bool cmp_by_time(const std::unique_ptr<PlayableEvent>& lhs, const std::unique_ptr<PlayableEvent>& rhs)
{
	return lhs->time() < rhs->time();
}
