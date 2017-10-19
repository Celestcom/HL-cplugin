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





std::vector<TimeOffset<ParameterizedEvent>> EventList::events()
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
	

	for (TimeOffset<ParameterizedEvent> event : source->m_events) {
		event.Time += offset;
		m_events.push_back(event);
	}
}

void EventList::Dupe(float offset)
{
	auto copy = m_events;
	for (auto event : copy) {
		event.Time += offset;
		m_events.push_back(event);

	}
}
