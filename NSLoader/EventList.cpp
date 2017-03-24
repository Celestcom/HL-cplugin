#include "stdafx.h"
#include "EventList.h"

#include "NSLoader.h"
#include "Locator.h"
EventList::EventList()
{
}

void EventList::AddEvent(NSVR_BasicHapticEvent_t * e)
{
	std::string result = Locator::getTranslator().ToString(e->Effect);
	BasicHapticEvent clone(e->Time, e->Strength, e->Duration, e->Area, result);
	
	//take a copy
	m_events.push_back(boost::variant<BasicHapticEvent>(std::move(clone)));
}


EventList::~EventList()
{
}

std::vector<boost::variant<BasicHapticEvent>> EventList::Events()
{
	return m_events;
}
