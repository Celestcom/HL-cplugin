#pragma once
#include "HLVR.h"

namespace hlvr {
namespace detail {
	struct event_create {
		event_create(HLVR_EventType type) : m_type{ type } {}
		HLVR_Result operator()(HLVR_Event** event) {
			return HLVR_Event_Create(event, m_type);
		}
		HLVR_EventType m_type;
	};
}}