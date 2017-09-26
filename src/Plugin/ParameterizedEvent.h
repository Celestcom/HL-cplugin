#pragma once
#include "NSLoader_fwds.h"
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <mutex>
#include <vector>
//#define DO_VALIDATION



//The possible attribute types that an event can have
typedef boost::variant<
	float, 
	int,
	std::vector<float>,
	std::vector<int>,
	std::vector<uint32_t>,
	std::vector<uint64_t>,
	uint32_t
> EventValue;


struct event_param {
	event_param();
	event_param(NSVR_EventKey key, EventValue);
	NSVR_EventKey key;
	EventValue value;
};


class ParameterizedEvent
{
public:
	explicit ParameterizedEvent(NSVR_EventType);
	//ParameterizedEvent(ParameterizedEvent&&);
//	ParameterizedEvent(const ParameterizedEvent&);
	template<class T>
	bool Set(NSVR_EventKey key, T value);
//	bool SetFloat(NSVR_EventKey key, float value);
//	bool SetInt(NSVR_EventKey key, int value);
	//bool SetFloats(NSVR_EventKey key, float* values, unsigned int length);
//	bool SetUInt32s(NSVR_EventKey key, uint32_t* values, unsigned int length);
	//bool SetUInt64s(NSVR_EventKey key, uint64_t* values, unsigned int length);
	template<typename ArrayType>
	bool Set(NSVR_EventKey key, ArrayType* values, unsigned int length);
	template<typename T>
	T GetOr(NSVR_EventKey key, T defaultValue) const;

	template<typename T>
	bool TryGet(NSVR_EventKey key, T* outVal) const;
	NSVR_EventType type() const;

private:
	NSVR_EventType m_type;
	std::vector<event_param> m_params;
	//std::mutex m_propLock;

	event_param* findParam(NSVR_EventKey key);
	const event_param* findParam(NSVR_EventKey key) const;
	
	template<typename T>
	void updateOrAdd(NSVR_EventKey key, T val);
};

template<class T>
inline bool ParameterizedEvent::Set(NSVR_EventKey key, T value)
{
//	std::lock_guard<std::mutex> guard(m_propLock);
	updateOrAdd<T>(key, value);
	return true;
}

template<typename ArrayType>
inline bool ParameterizedEvent::Set(NSVR_EventKey key, ArrayType * values, unsigned int length)
{
	std::vector<ArrayType> vec(values, values + length);
	updateOrAdd<std::vector<ArrayType>>(key, std::move(vec));
	return true;
}

template<typename T>
inline T ParameterizedEvent::GetOr(NSVR_EventKey key, T defaultValue) const
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
bool ParameterizedEvent::TryGet(NSVR_EventKey key, T* outVal) const
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
inline void ParameterizedEvent::updateOrAdd(NSVR_EventKey key, T val)
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
