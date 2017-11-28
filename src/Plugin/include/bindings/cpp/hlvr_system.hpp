#pragma once

#include <vector>
#include "HLVR.h"
#include "HLVR_Experimental.h"
#include "detail/hlvr_native_handle_owner.hpp"
#include "hlvr_event.hpp"
#include "hlvr_error.hpp"

#include "detail/expected.hpp"
#include <cassert>

namespace hlvr {

namespace detail {
using system_handle = native_handle_owner<HLVR_System, decltype(&HLVR_System_Create), decltype(&HLVR_System_Destroy)>;
}

struct nodes_t {};
struct regions_t {};

const static nodes_t nodes{};
const static regions_t regions{};


//tl::expected<system, status_code> make_system() {
//	HLVR_System* handle = nullptr;
//	HLVR_Result ec = HLVR_System_Create(&handle);
//	if (HLVR_FAIL(ec)) {
//		return tl::make_unexpected(status_code(ec));
//	}
//	else {
//		return 
//	}
//}
struct test {

};
system make_test() {
	HLVR_System* sys;
	return system(sys);
}
class system : public detail::system_handle {
public:
	system() : detail::system_handle { &HLVR_System_Create, &HLVR_System_Destroy } {}
	system(HLVR_System* raw) {}

	void shutdown() {
		m_handle.reset(nullptr);
	}

	status_code get_runtime_info(HLVR_RuntimeInfo* info) const {
		assert(m_handle);
		assert(info);
		return status_code(HLVR_System_GetRuntimeInfo(m_handle.get(), info));
	}

	std::vector<HLVR_DeviceInfo> get_known_devices() {
		assert(m_handle);

		std::vector<HLVR_DeviceInfo> devices;

		HLVR_DeviceIterator iter;
		HLVR_DeviceIterator_Init(&iter);

		while (HLVR_OK(HLVR_DeviceIterator_Next(&iter, m_handle.get()))) {
			devices.push_back(iter.DeviceInfo);
		}

		return devices;
	}

	std::vector<HLVR_NodeInfo> get_nodes(uint32_t device_id) {
		assert(m_handle);

		std::vector<HLVR_NodeInfo> nodes;

		HLVR_NodeIterator iter;
		HLVR_NodeIterator_Init(&iter);

		while (HLVR_OK(HLVR_NodeIterator_Next(&iter, device_id, m_handle.get()))) {
			nodes.push_back(iter.NodeInfo);
		}

		return nodes;
	}

	std::vector<HLVR_NodeInfo> get_all_nodes() {
		return get_nodes(hlvr_allnodes);
	}

	status_code push_event(hlvr::event& event) {
		assert(m_handle);
		assert(event);
		return status_code(HLVR_System_StreamEvent(m_handle.get(), event.native_handle()));
	}


	status_code poll_tracking(HLVR_TrackingUpdate* update) {
		assert(m_handle);
		assert(update);
		return status_code(HLVR_System_PollTracking(m_handle.get(), update));

	}

	status_code enable_tracking(uint32_t device_id) {
		assert(m_handle);
		return status_code(HLVR_System_EnableTracking(m_handle.get(), device_id));
	}

	status_code disable_tracking(uint32_t device_id) {
		assert(m_handle);
		return status_code(HLVR_System_DisableTracking(m_handle.get(), device_id));
	}

	friend system make_test();
};

}