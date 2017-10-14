#pragma once
#include "HLVR_Forwards.h"
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <mutex>
#include <vector>
//#define DO_VALIDATION



//The possible attribute types that an event can have
typedef boost::variant<
	float, 
	std::vector<float>,
	int,
	std::vector<int>,
	uint32_t,
	std::vector<uint32_t>,
	uint64_t,
	std::vector<uint64_t>
> EventValue;


struct event_param {
	event_param();
	event_param(HLVR_EventKey key, EventValue);
	HLVR_EventKey key;
	EventValue value;
};


class ParameterizedEvent
{
public:
	explicit ParameterizedEvent();

	template<class T>
	bool Set(HLVR_EventKey key, T value);

	template<typename ArrayType>
	bool Set(HLVR_EventKey key, const ArrayType* values, unsigned int length);
	template<typename T>
	T GetOr(HLVR_EventKey key, T defaultValue) const;

	template<typename T>
	bool TryGet(HLVR_EventKey key, T* outVal) const;
	HLVR_EventType type() const;

	bool HasKey(HLVR_EventKey key) const;

	void setType(HLVR_EventType type);
private:
	HLVR_EventType m_type;
	std::vector<event_param> m_params;

	event_param* findParam(HLVR_EventKey key);
	const event_param* findParam(HLVR_EventKey key) const;
	
	template<typename T>
	void updateOrAdd(HLVR_EventKey key, T val);
};

template<class T>
inline bool ParameterizedEvent::Set(HLVR_EventKey key, T value)
{
//	std::lock_guard<std::mutex> guard(m_propLock);
	updateOrAdd<T>(key, value);
	return true;
}

template<typename ArrayType>
inline bool ParameterizedEvent::Set(HLVR_EventKey key, const ArrayType * values, unsigned int length)
{
	std::vector<ArrayType> vec(values, values + length);
	updateOrAdd<std::vector<ArrayType>>(key, std::move(vec));
	return true;
}


template<typename T>
inline T ParameterizedEvent::GetOr(HLVR_EventKey key, T defaultValue) const
{
	try {
		if (const event_param* prop = findParam(key)) {
			return boost::get<T>(prop->value);
		}
		else {
			return defaultValue;
		}
	}
	catch (const boost::bad_get&) {
		return defaultValue;
	}
}

template<typename T>
bool ParameterizedEvent::TryGet(HLVR_EventKey key, T* outVal) const
{
	try {
		if (const event_param* prop = findParam(key)) {
			*outVal = boost::get<T>(prop->value);
			return true;
		}
		else {
			*outVal = T();
			return false;
		}
	}
	catch (const boost::bad_get&) {
		*outVal = T();
		return false;
	}
}

template<typename T>
inline void ParameterizedEvent::updateOrAdd(HLVR_EventKey key, T val)
{
	if (event_param* existing = findParam(key)) {
		existing->value = val;
	}
	else {
		m_params.emplace_back(key, val);
	}
}



template<class T>
inline T minimum_bound(T minimum, T value) {
	return std::max<T>(minimum, value);
}

template<class T>
inline T maximum_bound(T max, T value) {
	return std::min<T>(max, value);
}

template<class T>
inline T constrain(T min, T val, T max) {
	return std::clamp<T>(val, min, max);
}
