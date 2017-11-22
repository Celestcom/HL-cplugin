#pragma once

#include "HLVR.h"
#include "detail/hlvr_native_handle_owner.hpp"

namespace hlvr 
{


using timeline_handle = detail::native_handle_owner<HLVR_Timeline, decltype(&HLVR_Timeline_Create), decltype(&HLVR_Timeline_Destroy)>;
class timeline : public timeline_handle {
public:
	timeline() : timeline_handle{ &HLVR_Timeline_Create, &HLVR_Timeline_Destroy } {}


	bool add_event(hlvr::event& ev, double timeOffsetFractionalSeconds) {
		assert(m_handle);
		m_lastError = HLVR_Timeline_AddEvent(m_handle.get(), timeOffsetFractionalSeconds, ev.native_handle());
		return HLVR_OK(m_lastError);
	}

	bool transmit(hlvr::system& system, hlvr::effect& effect) {
		assert(m_handle);
		assert(system);
		assert(effect);

		m_lastError = HLVR_Timeline_Transmit(m_handle.get(), system.native_handle(), effect.native_handle());
		return HLVR_OK(m_lastError);
	}

};

}