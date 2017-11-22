#pragma once

#include "HLVR.h"
#include "detail/hlvr_native_handle_owner.hpp"
#include "detail/hlvr_event_create.hpp"
namespace hlvr 
{

using event_handle = detail::native_handle_owner<HLVR_Event, detail::event_create, decltype(&HLVR_Event_Destroy)>;
class event : public event_handle {
public:
	explicit event(HLVR_EventType type) : event_handle{ detail::event_create(type), &HLVR_Event_Destroy } {}



	bool set_float(HLVR_EventKey key, float val) {
		assert(m_handle);
		m_lastError = HLVR_Event_SetFloat(m_handle.get(), key, val);
		return HLVR_OK(m_lastError);
	}

	bool set_floats(HLVR_EventKey key, const float* values, uint32_t length) {
		assert(m_handle);
		m_lastError = HLVR_Event_SetFloats(m_handle.get(), key, values, length);
		return HLVR_OK(m_lastError);
	}

	bool set_uint32(HLVR_EventKey key, uint32_t val) {
		assert(m_handle);
		m_lastError = HLVR_Event_SetUInt32(m_handle.get(), key, val);
		return HLVR_OK(m_lastError);
	}
	bool set_uint32s(HLVR_EventKey key, const uint32_t* values, uint32_t length) {
		assert(m_handle);
		assert(values != nullptr);
		m_lastError = HLVR_Event_SetUInt32s(m_handle.get(), key, values, length);
		return HLVR_OK(m_lastError);
	}

	bool set_int(HLVR_EventKey key, int val) {
		assert(m_handle);
		m_lastError = HLVR_Event_SetInt(m_handle.get(), key, val);
		return HLVR_OK(m_lastError);
	}


};



}