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

template<typename T, typename Creator, typename Deleter>
class native_handle_owner {
public:
	explicit native_handle_owner(Creator c, Deleter d)
		: m_handle{ nullptr, d }
		, m_lastError{ 0 }
	{
		T* tempHandle = nullptr;
		m_lastError = c(&tempHandle);

		if (HLVR_OK(m_lastError)) {
			m_handle.reset(tempHandle);
		}
	}

	const T* native_handle() const {
		return m_handle.get();
	}

	T* native_handle() {
		return m_handle.get();
	}

	explicit operator bool() const noexcept {
		return (bool)m_handle;
	}

	HLVR_Result last_error() const noexcept {
		return m_lastError;
	}
protected:
	std::unique_ptr<T, Deleter> m_handle;
	HLVR_Result m_lastError;
};


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

using system_handle = native_handle_owner<HLVR_System, decltype(&HLVR_System_Create), decltype(&HLVR_System_Destroy)>;
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

	bool push_event(HLVR_Event* event) {
		assert(m_handle);
		assert(event != nullptr);
		m_lastError = HLVR_System_StreamEvent(m_handle.get(), event);
		return HLVR_OK(m_lastError);
	}

	
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
		HLVR_Event_Destroy(m_event);
	}

	event(event&& other) : m_event{ other.m_event }, m_lastError{ other.m_lastError } {
		other.m_event = nullptr;
	}
	event& operator=(event&& other) {
		HLVR_Event_Destroy(m_event);
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
	timeline() 
		: m_timeline{ nullptr, [](HLVR_Timeline* t) { HLVR_Timeline_Destroy(t); } }
		, m_lastError{ 0 } 
	{
		HLVR_Timeline* tempHandle = nullptr;
		m_lastError = HLVR_Timeline_Create(&tempHandle);

		if (HLVR_OK(m_lastError)) {
			m_timeline.reset(tempHandle);
		}
	}
	
	HLVR_Result last_error() const {
		return m_lastError;
	}

	
	explicit operator bool() const noexcept {
		return (bool) m_timeline;
	}

	bool add_event(hlvr::event& ev, double timeOffsetFractionalSeconds) {
		assert(m_timeline != nullptr);
		m_lastError = HLVR_Timeline_AddEvent(m_timeline.get(), timeOffsetFractionalSeconds, ev.native_handle());
		return HLVR_OK(m_lastError);
	}

	HLVR_Timeline* native_handle() {
		return m_timeline.get();
	}

	const HLVR_Timeline* native_handle() const {
		return m_timeline.get();
	}
private:
	std::unique_ptr<HLVR_Timeline, decltype(&HLVR_Timeline_Destroy)> m_timeline;
	HLVR_Result m_lastError;
};





using effect_handle = native_handle_owner<HLVR_Effect, decltype(&HLVR_Effect_Create), decltype(&HLVR_Effect_Destroy)>;

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


HLVR_Result transmit(system& system, timeline& timeline, effect& effect) {
	return HLVR_Timeline_Transmit(timeline.native_handle(), system.native_handle(), effect.native_handle());
}


}