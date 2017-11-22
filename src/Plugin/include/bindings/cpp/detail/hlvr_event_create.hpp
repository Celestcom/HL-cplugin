#pragma once
#include "HLVR.h"

namespace hlvr {
namespace detail {
	struct event_create {
		event_create(HLVR_EventType t) : m_t{ t } {}
		HLVR_Result operator()(HLVR_Event** e) {
			return HLVR_Event_Create(e, m_t);
		}
		HLVR_EventType m_t;
	};
}}