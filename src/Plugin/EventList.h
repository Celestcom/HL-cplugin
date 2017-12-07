#pragma once

#include "ParameterizedEvent.h"

#include <mutex>

template<typename T>
struct TimeOffset {
	float Time;
	T Data;
};

class EventList
{
public:
	
	EventList();
	int AddEvent(TimeOffset<TypedEvent> data);
	std::vector<TimeOffset<TypedEvent>> events() const;
	bool empty() const;
private:
	std::vector<TimeOffset<TypedEvent>> m_events;
	mutable std::mutex m_eventLock;

};

