#pragma once

#include <boost\variant.hpp>
#include "Wire\IntermediateHapticFormats.h"
#include "Event.h"

class ParameterizedEvent;
class Engine;

class EventList
{
public:
	EventList(Engine* enginePtr);
	int AddEvent(ParameterizedEvent* e);
	~EventList();
	Engine* EnginePtr();
	std::vector<boost::variant<BasicHapticEvent>> Events();
private:
	std::vector<boost::variant<BasicHapticEvent>> m_events;
	Engine* m_engine;
};

