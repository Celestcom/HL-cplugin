#pragma once

#include <boost\variant.hpp>
#include "NSLoader_fwds.h"
#include "Event.h"
class EventList
{
public:
	EventList();
	int AddEvent(Event* e);
	~EventList();

	std::vector<boost::variant<BasicHapticEvent>> Events();
private:
	std::vector<boost::variant<BasicHapticEvent>> m_events;
};

