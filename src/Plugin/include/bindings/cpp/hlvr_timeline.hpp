#pragma once


#include "detail/hlvr_native_handle_owner.hpp"

#include "hlvr_event.hpp"
#include "hlvr_system.hpp"
#include "hlvr_effect.hpp"
#include "hlvr_error.hpp"

#include "HLVR.h"
#include <cassert>

namespace hlvr {

namespace detail {
using timeline_handle = native_handle_owner<HLVR_Timeline, decltype(&HLVR_Timeline_Create), decltype(&HLVR_Timeline_Destroy)>;
}

class timeline : public detail::timeline_handle {
public:
	timeline() : detail::timeline_handle{ &HLVR_Timeline_Create, &HLVR_Timeline_Destroy } {}


	status_code add_event(const hlvr::event& ev, double timeOffsetFractionalSeconds) {
		assert(m_handle);
		return status_code(HLVR_Timeline_AddEvent(m_handle.get(), timeOffsetFractionalSeconds, ev.native_handle()));
	}

	status_code transmit(hlvr::system& system, hlvr::effect& effect) const {
		assert(m_handle);
		assert(system);
		assert(effect);

		return status_code(HLVR_Timeline_Transmit(m_handle.get(), system.native_handle(), effect.native_handle()));
	}

};

}