#include "stdafx.h"
#include "EventList.h"

#include "NSLoader.h"
#include "Locator.h"
#include "ParameterizedEvent.h"
#include "Engine.h"

EventList::EventList()
{
}

int EventList::AddEvent(ParameterizedEvent * pe)
{	
	if (pe == nullptr) {
		return -1;
	}
	BasicHapticEvent* e = static_cast<BasicHapticEvent*>(pe);

	//take a copy
	m_events.push_back(boost::variant<BasicHapticEvent>(*e));
	return 1;
}


EventList::~EventList()
{
}



std::vector<boost::variant<BasicHapticEvent>> EventList::Events()
{
	return m_events;
}
