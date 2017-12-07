#include "stdafx.h"
#include "ParameterizedEvent.h"
#include "HLVR.h"


TypedEvent::TypedEvent(HLVR_EventType type) 
	: Type(type)
	, Params() {}

ParameterizedEvent::ParameterizedEvent()
	: m_params()
{
	//Trying to avoid a resize
	m_params.reserve(5);
}



bool ParameterizedEvent::HasKey(HLVR_EventKey key) const
{
	return findParam(key) != nullptr;
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



event_param::event_param(HLVR_EventKey key, EventValue val)
	: key(key)
	, value(val)
{
}
