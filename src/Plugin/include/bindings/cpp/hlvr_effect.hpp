#pragma once

#include "detail/hlvr_native_handle_owner.hpp"
#include "hlvr_error.hpp"
#include "HLVR.h"
#include <cassert>

namespace hlvr {
namespace detail {
using effect_handle = native_handle_owner<HLVR_Effect, decltype(&HLVR_Effect_Create), decltype(&HLVR_Effect_Destroy)>;
}

class effect : public detail::effect_handle {
public:
	effect() : detail::effect_handle{ &HLVR_Effect_Create, &HLVR_Effect_Destroy } {}

	status_code play() {
		assert(m_handle);
		return status_code(HLVR_Effect_Play(m_handle.get()));
	}

	status_code pause() {
		assert(m_handle);
		return status_code(HLVR_Effect_Pause(m_handle.get()));
	}

	status_code reset() {
		assert(m_handle);
		return status_code(HLVR_Effect_Reset(m_handle.get()));
	}
};
}