#include "stdafx.h"
#include "EventList.h"

#include "ParameterizedEvent.h"
#include "HLVR_Errors.h"
#include "HLVR.h"
EventList::EventList():m_events(), m_eventLock()
{
	m_events.reserve(1);
}

//Precondition: event is not null
int EventList::AddEvent(TimeOffset<ParameterizedEvent> event)
{	
	std::lock_guard<std::mutex> guard(m_eventLock);
	m_events.push_back(std::move(event));
	
	return HLVR_Ok;
}

std::vector<TimeOffset<ParameterizedEvent>> EventList::events() const
{
	std::lock_guard<std::mutex> guard(m_eventLock);
	return m_events;
}

bool EventList::empty() const
{
	std::lock_guard<std::mutex> guard(m_eventLock);
	return m_events.empty();
}

