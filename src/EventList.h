#pragma once

#include <boost\variant.hpp>
#include "BasicHapticEvent.h"
class ParameterizedEvent;
class Engine;

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

