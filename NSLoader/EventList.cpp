#include "stdafx.h"
#include "EventList.h"

#include "NSLoader.h"
#include "Locator.h"
#include "Event.h"
#include "ParameterizedEvent.h"
#include "Engine.h"
EventList::EventList(Engine* enginePtr):m_engine(enginePtr)
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

Engine * EventList::EnginePtr()
{
	return m_engine;
}

std::vector<boost::variant<BasicHapticEvent>> EventList::Events()
{
	return m_events;
}
