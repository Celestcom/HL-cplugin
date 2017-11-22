#pragma once

#include "HLVR.h"
#include "detail/hlvr_native_handle_owner.hpp"

namespace hlvr 
{


using effect_handle = detail::native_handle_owner<HLVR_Effect, decltype(&HLVR_Effect_Create), decltype(&HLVR_Effect_Destroy)>;

class effect : public effect_handle {
public:
	effect() : effect_handle{ &HLVR_Effect_Create, &HLVR_Effect_Destroy } {}


	bool play() {
		assert(m_handle);
		m_lastError = HLVR_Effect_Play(m_handle.get());
		return HLVR_OK(m_lastError);
	}

	bool pause() {
		assert(m_handle);
		m_lastError = HLVR_Effect_Pause(m_handle.get());
		return HLVR_OK(m_lastError);
	}

	bool reset() {
		assert(m_handle);
		m_lastError = HLVR_Effect_Reset(m_handle.get());
		return HLVR_OK(m_lastError);
	}

};


}