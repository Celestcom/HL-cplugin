#include "stdafx.h"
#include "EventList.h"

#include "ParameterizedEvent.h"
#include "NSLoader_Errors.h"
#include "NSLoader.h"
EventList::EventList():m_events(), m_eventLock()
{
	m_events.reserve(1);
}

//Precondition: event is not null
int EventList::AddEvent(ParameterizedEvent * event)
{	
	std::lock_guard<std::mutex> guard(m_eventLock);
	m_events.push_back(*event);
	
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

void EventList::Interleave(EventList* source, float offset)
{
	

	for (ParameterizedEvent event : source->m_events) {
		event.Set(NSVR_EventKey_Time_Float, event.Get(NSVR_EventKey_Time_Float, 0.0f) + offset);
		m_events.push_back(event);
	}
	//m_events.insert(m_events.end(), source->m_events.begin(), source->m_events.end());
}

void EventList::Dupe(float offset)
{
	auto copy = m_events;
	for (auto event : copy) {
		event.Set(NSVR_EventKey_Time_Float, event.Get(NSVR_EventKey_Time_Float, 0.0f) + offset);
		m_events.push_back(event);

	}
}
