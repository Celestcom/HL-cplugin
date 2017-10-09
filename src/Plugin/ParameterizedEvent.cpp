#include "stdafx.h"
#include "ParameterizedEvent.h"
#include "HLVR.h"
#include "BasicHapticEvent.h"

ParameterizedEvent::ParameterizedEvent(HLVR_EventType type): 
	m_type(type),
	m_params()
{
	//average event key count is 5, so preemptively reserve that much
	//change if proves unnecessary
	m_params.reserve(5);
}

HLVR_EventType ParameterizedEvent::type() const
{
	return m_type;
}

event_param* ParameterizedEvent::findParam(HLVR_EventKey key)
{
	return const_cast<event_param*>(
		static_cast<const ParameterizedEvent*>(this)->findParam(key)
	);
}

const event_param * ParameterizedEvent::findParam(HLVR_EventKey key) const
{
	for (const auto& param : m_params) {
		if (param.key == key) {
			return &param;
		}
	}
	return nullptr;
}

event_param::event_param(): 
	key(), 
	value()
{
}

event_param::event_param(HLVR_EventKey key, EventValue val):
	key(key),
	value(val)
{
}
