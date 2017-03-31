#include "stdafx.h"
#include "Event.h"


Event::Event(NSVR_EventType eventType)
{
	switch (eventType) {
	case NSVR_EventType::NSVR_EventType_BasicHapticEvent:
		m_event = boost::variant<BasicHapticEvent>(BasicHapticEvent());
		break;
	default:
		break;
	}
}


Event::~Event()
{
}

bool Event::SetFloat(const char * key, float f)
{
	SetParamVisitor<float> visitor(key, f);
	return boost::apply_visitor(visitor, m_event);
}

bool Event::SetInteger(const char * key, int i)
{
	SetParamVisitor<int> visitor(key, i);
	return boost::apply_visitor(visitor, m_event);
}

boost::variant<BasicHapticEvent> Event::GetEvent()
{
	return m_event;
}

