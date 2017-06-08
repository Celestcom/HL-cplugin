#pragma once

#include <memory>

class ParameterizedEvent;
class Engine;

class EventList
{
public:
	EventList();
	int AddEvent(ParameterizedEvent* e);
	~EventList();
	const std::vector<std::unique_ptr<ParameterizedEvent>>& events();
private:
	std::vector<std::unique_ptr<ParameterizedEvent>> m_events;
};

