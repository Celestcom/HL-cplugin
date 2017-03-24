#include "stdafx.h"
#include "EventList.h"
#include "Wire/IntermediateHapticFormats.h"

#include "NSLoader.h"
#include "Locator.h"
#include "Event.h"
EventList::EventList()
{
}

int EventList::AddEvent(Event * e)
{	
	if (e == nullptr) {
		return -1;
	}
	//take a copy
	m_events.push_back(e->GetEvent());
	return 1;
}


EventList::~EventList()
{
}

std::vector<boost::variant<BasicHapticEvent>> EventList::Events()
{
	return m_events;
}
