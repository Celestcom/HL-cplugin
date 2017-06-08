#include "stdafx.h"
#include "EventList.h"

#include "ParameterizedEvent.h"
#include "NSLoader_Errors.h"
EventList::EventList()
{
}

//Precondition: event is not null
int EventList::AddEvent(ParameterizedEvent * event)
{	
	auto clone = event->Clone();
	m_events.push_back(std::move(clone));
	
	return NSVR_Success_Unqualified;
}


EventList::~EventList()
{
}



const std::vector<std::unique_ptr<ParameterizedEvent>>& EventList::events()
{
	return m_events;
}
