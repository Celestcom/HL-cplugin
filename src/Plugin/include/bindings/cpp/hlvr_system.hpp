#pragma once

#include "HLVR.h"

namespace hlvr 
{

class system {
public:
	system() : m_system{nullptr}, m_created{false} {}
	~system() {
		if (m_created) {
			destroy();
		}
	}
	system(const system&) = delete;
	system& operator=(const system&) = delete;

	bool create() {
		if (m_created) { return true; }

		auto ec = HLVR_System_Create(&m_system, nullptr);
		m_created = HLVR_OK(ec);
		return m_created;
	}

	void destroy() {
		if (!m_created) { return; }

		HLVR_System_Destroy(&m_system);

		m_created = false;

	}

	bool get_runtime_info(HLVR_RuntimeInfo* info) {
		assert(m_created);
		assert(info != nullptr);
		auto ec = HLVR_System_GetRuntimeInfo(m_system, info);
		return HLVR_OK(ec);
	}

	bool poll_tracking(HLVR_TrackingUpdate* update) {
		assert(m_created);
		assert(update != nullptr);
		auto ec = HLVR_System_PollTracking(m_system, update);
		return HLVR_OK(ec);
	}

	void push_event(HLVR_Event* event) {
		assert(m_created);
		assert(event != nullptr);
		HLVR_System_StreamEvent(m_system, event);
	}

	HLVR_System* native_handle() { return m_system; }
private:
	HLVR_System* m_system;
	bool m_created;
};

}