#pragma once

#include "Wire/IntermediateHapticFormats.h"
#include <boost\variant.hpp>
#include "NSLoader_fwds.h"
class EventList
{
public:
	EventList();
	void AddEvent(NSVR_BasicHapticEvent_t* e);
	~EventList();

	std::vector<boost::variant<BasicHapticEvent>> Events();
private:
	std::vector<boost::variant<BasicHapticEvent>> m_events;
};

