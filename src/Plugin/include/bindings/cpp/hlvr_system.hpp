#pragma once

#include "HLVR.h"
#include "detail/hlvr_native_handle_owner.hpp"
#include "hlvr_event.hpp"

namespace hlvr 
{

struct nodes_t {};
struct regions_t {};

const static nodes_t nodes{};
const static regions_t regions{};

using system_handle = detail::native_handle_owner<HLVR_System, decltype(&HLVR_System_Create), decltype(&HLVR_System_Destroy)>;
class system : public system_handle {
public:
	system() : system_handle { &HLVR_System_Create, &HLVR_System_Destroy } {}
	

	void shutdown() {
		m_handle.reset(nullptr);
	}

	bool get_runtime_info(HLVR_RuntimeInfo* info) {
		assert(m_handle);
		assert(info != nullptr);
		m_lastError = HLVR_System_GetRuntimeInfo(m_handle.get(), info);
		return HLVR_OK(m_lastError);
	}

	bool poll_tracking(HLVR_TrackingUpdate* update) {
		assert(m_handle);
		assert(update != nullptr);
		m_lastError = HLVR_System_PollTracking(m_handle.get(), update);
		return HLVR_OK(m_lastError);
	}

	bool push_event(hlvr::event& event) {
		assert(m_handle);
		assert((bool)event);
		m_lastError = HLVR_System_StreamEvent(m_handle.get(), event.native_handle());
		return HLVR_OK(m_lastError);
	}
};

}