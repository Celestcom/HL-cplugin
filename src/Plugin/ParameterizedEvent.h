#pragma once
#include "HLVR_Forwards.h"
#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <mutex>
#include <vector>

//ParameterizedEvent's purpose is to hold a bunch of attributes in a generic fashion. 
//For example, we might define a HapticEvent with parameters "duration:float", "name:string".

//Here we list the different types of parameters that can be held.
//If you need to introduce a new type, this is where that would happen.
using EventValue = boost::variant<
	float,
	std::vector<float>,

	int,
	std::vector<int>,

	uint32_t,
	std::vector<uint32_t>,

	uint64_t,
	std::vector<uint64_t>
>;

//Glue an EventValue to a specific key - an instantiation of a parameter.
//Example: key "duration" is bound to value 3.0f;
struct event_param {
	event_param() = default;
	event_param(HLVR_EventKey key, EventValue value);
	HLVR_EventKey key;
	EventValue value;
};


class ParameterizedEvent
{
public:
	ParameterizedEvent();

	template<class T>
	bool Set(HLVR_EventKey key, T value);

	template<typename ArrayType>
	bool Set(HLVR_EventKey key, const ArrayType* values, unsigned int length);

	template<typename T>
	T GetOr(HLVR_EventKey key, T defaultValue) const;

	template<typename T>
	boost::optional<T> TryGet(HLVR_EventKey key) const;

	template<typename T>
	bool TryGet(HLVR_EventKey key, T* outVal) const;

	bool HasKey(HLVR_EventKey key) const;

private:
	std::vector<event_param> m_params;

	event_param* findParam(HLVR_EventKey key);
	const event_param* findParam(HLVR_EventKey key) const;
	
	template<typename T>
	void updateOrAdd(HLVR_EventKey key, T val);
};

struct TypedEvent {
	explicit TypedEvent(HLVR_EventType type);
	HLVR_EventType Type;
	ParameterizedEvent Params;

};

template<class T>
inline bool ParameterizedEvent::Set(HLVR_EventKey key, T value)
{
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

//TryGet will not modify outVal if it fails to get the value
template<typename T>
boost::optional<T> ParameterizedEvent::TryGet(HLVR_EventKey key) const
{
	try {
		if (const event_param* prop = findParam(key)) {
			return boost::get<T>(prop->value)
		}
	}
	catch (const boost::bad_get&) {
		//noop
	}

	return boost::none;
}


template<typename T>
bool ParameterizedEvent::TryGet(HLVR_EventKey key, T* outVal) const {
	try {
		if (const event_param* prop = findParam(key)) {
			*outVal = boost::get<T>(prop->value);
			return true;
		}
	}
	catch (const boost::bad_get&) {
		//noop
	}

	return false;
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
