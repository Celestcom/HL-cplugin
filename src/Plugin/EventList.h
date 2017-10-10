#pragma once

#include <memory>
#include <mutex>
#include "ParameterizedEvent.h"
class Engine;

struct TimeAndType {
	float TimeOffset;
	HLVR_EventType Type;
	ParameterizedEvent Data;
};

class EventList
{
public:
	
	EventList();
	int AddEvent(TimeAndType data);
	std::vector<TimeAndType> events();
	bool empty() const;
private:
	std::vector<TimeAndType> m_events;
	std::mutex m_eventLock;
public:
	void Interleave(EventList* source, float offset);
	void Dupe(float offset);
};

