#pragma once

#include <boost\variant.hpp>
#include "NSLoader_fwds.h"
#include "Event.h"

class ParameterizedEvent;


class EventList
{
public:
	EventList();
	int AddEvent(ParameterizedEvent* e);
	~EventList();

	std::vector<boost::variant<BasicHapticEvent>> Events();
private:
	std::vector<boost::variant<BasicHapticEvent>> m_events;
};

