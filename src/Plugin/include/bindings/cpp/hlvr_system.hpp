#pragma once

#include "HLVR.h"
#include <memory>
namespace hlvr 
{

template<class T>
using owner = T;


struct nodes_t {};
struct regions_t {};

const static nodes_t nodes{};
const static regions_t regions{};

const char* error_c_str(HLVR_Result result) {
		switch (result) {
		case HLVR_Error_UNKNOWN:
			return "Unknown";
		case HLVR_Error_Unspecified:
			return "Unspecified";
		case HLVR_Error_NullArgument:
			return "NullArgument";
		case HLVR_Error_InvalidArgument:
			return "InvalidArgument";
		case HLVR_Error_InvalidEventType:
			return "InvalidEventType";
		case HLVR_Error_NotConnected:
			return "NotConnected";
		case HLVR_Error_NoSuchHandle:
			return "NoSuchHandle";
		case HLVR_Error_NoMoreDevices:
			return "NoMoreDevices";
		case HLVR_Error_NoMoreNodes:
			return "NoMoreNodes";
		case HLVR_Error_InvalidTimeOffset:
			return "InvalidTimeOffset";
		case HLVR_Error_EmptyTimeline:
			return "EmptyTimeline";
		default:
			return "Unknown";
		}
	
}
class system {
public:
	system() : m_system{ nullptr }, m_lastError(0) {

		m_lastError = HLVR_System_Create(&m_system, nullptr);
		if (HLVR_FAIL(m_lastError)) {
			m_system = nullptr;
		}
	}
	~system() {
		shutdown();
	}
	system(const system&) = delete;
	system& operator=(const system&) = delete;

	explicit operator bool() const noexcept {
		return m_system != nullptr;
	}

	HLVR_Result last_error() const {
		return m_lastError;
	}

	

	void provide_native_handle(owner<HLVR_System*> handle) {
		m_system = handle;
	}

	void shutdown() {
		if (m_system != nullptr) {
			HLVR_System_Destroy(&m_system);
			m_system = nullptr;
		}
	}

	bool get_runtime_info(HLVR_RuntimeInfo* info) {
		assert(m_system != nullptr);
		assert(info != nullptr);
		m_lastError = HLVR_System_GetRuntimeInfo(m_system, info);
		return HLVR_OK(m_lastError);
	}

	bool poll_tracking(HLVR_TrackingUpdate* update) {
		assert(m_system != nullptr);
		assert(update != nullptr);
		m_lastError = HLVR_System_PollTracking(m_system, update);
		return HLVR_OK(m_lastError);
	}

	bool push_event(HLVR_Event* event) {
		assert(m_system != nullptr);
		assert(event != nullptr);
		m_lastError = HLVR_System_StreamEvent(m_system, event);
		return HLVR_OK(m_lastError);
	}

	//returns a non-owning handle
	HLVR_System* native_handle() { return m_system; }

private:
	owner<HLVR_System*> m_system;
	HLVR_Result m_lastError;
};



class event {
public:
	explicit event(HLVR_EventType type) : m_event{ nullptr }, m_lastError{ 0 } {
		m_lastError = HLVR_Event_Create(&m_event, type);
		if (HLVR_FAIL(m_lastError)) {
			m_event = nullptr;
		}
	}

	~event() {
		HLVR_Event_Destroy(&m_event);
	}

	event(event&& other) : m_event{ other.m_event }, m_lastError{ other.m_lastError } {
		other.m_event = nullptr;
	}
	event& operator=(event&& other) {
		HLVR_Event_Destroy(&m_event);
		m_event = other.m_event;
		other.m_event = nullptr;
		return *this;
	}
	HLVR_Result last_error() const {
		return m_lastError;
	}
	event(const event&) = delete;
	event& operator=(const event&) = delete;

	HLVR_Event* native_handle() {
		return m_event;
	}

	const HLVR_Event* native_handle() const {
		return m_event;
	}
	explicit operator bool() const noexcept {
		return m_event != nullptr;
	}

	bool set_float(HLVR_EventKey key, float val) {
		assert(m_event != nullptr);
		m_lastError = HLVR_Event_SetFloat(m_event, key, val);
		return HLVR_OK(m_lastError);
	}

	bool set_floats(HLVR_EventKey key, const float* values, uint32_t length) {
		assert(m_event != nullptr);
		m_lastError = HLVR_Event_SetFloats(m_event, key, values, length);
		return HLVR_OK(m_lastError);
	}

	bool set_uint32(HLVR_EventKey key, uint32_t val) {
		assert(m_event != nullptr);
		m_lastError = HLVR_Event_SetUInt32(m_event, key, val);
		return HLVR_OK(m_lastError);
	}
	bool set_uint32s(HLVR_EventKey key, const uint32_t* values, uint32_t length) {
		assert(m_event != nullptr);
		assert(values != nullptr);
		m_lastError = HLVR_Event_SetUInt32s(m_event, key, values, length);
		return HLVR_OK(m_lastError);
	}

	bool set_int(HLVR_EventKey key, int val) {
		assert(m_event != nullptr);
		m_lastError = HLVR_Event_SetInt(m_event, key, val);
		return HLVR_OK(m_lastError);
	}

private:
	owner<HLVR_Event*> m_event;
	HLVR_Result m_lastError;
};

class timeline {
public:
	timeline() : m_timeline{ nullptr }, m_lastError{ 0 } {
		m_lastError = HLVR_Timeline_Create(&m_timeline);
		if (HLVR_FAIL(m_lastError)) {
			m_timeline = nullptr;
		}
	}
	~timeline() {
		HLVR_Timeline_Destroy(&m_timeline);
	}

	timeline(const timeline&) = delete;
	timeline& operator=(const timeline&) = delete;
	HLVR_Result last_error() const {
		return m_lastError;
	}

	
	explicit operator bool() const noexcept {
		return m_timeline != nullptr;
	}

	bool add_event(hlvr::event& ev, double timeOffsetFractionalSeconds) {
		assert(m_timeline != nullptr);
		m_lastError = HLVR_Timeline_AddEvent(m_timeline, timeOffsetFractionalSeconds, ev.native_handle());
		return HLVR_OK(m_lastError);
	}

	HLVR_Timeline* native_handle() {
		return m_timeline;
	}

	const HLVR_Timeline* native_handle() const {
		return m_timeline;
	}
private:
	owner<HLVR_Timeline*> m_timeline;
	HLVR_Result m_lastError;
};


class effect {
public:
	effect() : m_effect{ nullptr }, m_lastError{ 0 } {
		m_lastError = HLVR_Effect_Create(&m_effect);
		if (HLVR_FAIL(m_lastError)) {
			m_effect = nullptr;
		}
	}

	~effect() {
		HLVR_Effect_Destroy(&m_effect);
	}

	HLVR_Effect* native_handle() {
		return m_effect;
	}

	effect(const effect&) = delete;
	effect& operator=(const effect&) = delete;


	bool play() {
		assert(m_effect != nullptr);
		m_lastError = HLVR_Effect_Play(m_effect);
		return HLVR_OK(m_lastError);
	}

private:
	std::unique_ptr<HLVR_Effect> m_effect;
	HLVR_Result  m_lastError;
};


HLVR_Result transmit(system& system, timeline& timeline, effect& effect) {
	return HLVR_Timeline_Transmit(timeline.native_handle(), system.native_handle(), effect.native_handle());
}


}