#pragma once

#include <memory>
#include <mutex>
class ParameterizedEvent;
class Engine;

class EventList
{
public:
	EventList();
	int AddEvent(ParameterizedEvent* e);
	~EventList();
	const std::vector<ParameterizedEvent>& events();
private:
	std::vector<ParameterizedEvent> m_events;
	std::mutex m_eventLock;
};

