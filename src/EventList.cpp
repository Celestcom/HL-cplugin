#include "stdafx.h"
#include "EventList.h"

#include "ParameterizedEvent.h"
#include "NSLoader_Errors.h"

EventList::EventList():m_events()
{
}

//Precondition: event is not null
int EventList::AddEvent(ParameterizedEvent * event)
{	
	m_events.push_back(std::make_unique<ParameterizedEvent>(*event));
	
	return NSVR_Success_Unqualified;
}


EventList::~EventList()
{
}



const std::vector<std::unique_ptr<ParameterizedEvent>>& EventList::events()
{
	return m_events;
}
