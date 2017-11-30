#pragma once

#include <memory>
#include <mutex>
#include "ParameterizedEvent.h"
class Engine;



template<typename T>
struct TimeOffset {
	float Time;
	T Data;
};

class EventList
{
public:
	
	EventList();
	int AddEvent(TimeOffset<ParameterizedEvent> data);
	std::vector<TimeOffset<ParameterizedEvent>> events() const;
	bool empty() const;
private:
	std::vector<TimeOffset<ParameterizedEvent>> m_events;
	mutable std::mutex m_eventLock;

};

