#include "stdafx.h"
#include "EventList.h"

#include "ParameterizedEvent.h"
#include "NSLoader_Errors.h"

EventList::EventList():m_events(), m_eventLock()
{
}

//Precondition: event is not null
int EventList::AddEvent(ParameterizedEvent * event)
{	
	std::lock_guard<std::mutex> guard(m_eventLock);
	m_events.push_back(ParameterizedEvent(*event));
	
	return NSVR_Success_Unqualified;
}


EventList::~EventList()
{
}



std::vector<ParameterizedEvent> EventList::events()
{
	std::lock_guard<std::mutex> guard(m_eventLock);

	return m_events;
}

bool EventList::empty() const
{
	return m_events.empty();
}
