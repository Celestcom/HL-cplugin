#pragma once
#include "NSLoader_fwds.h"
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <mutex>
#include <vector>
//#define DO_VALIDATION

#ifdef DO_VALIDATION
#define VALIDATE_KEY(key) do { if (!validate(key)) { return false; } } while (0)
#else 
#define VALIDATE_KEY(key)
#endif


//The possible attribute types that an event can have
typedef boost::variant<
	float, 
	int,
	std::vector<float>,
	std::vector<int>
> EventValue;


struct event_param {
	event_param();
	event_param(const char* key, EventValue);
	std::string key;
	EventValue value;
};


class ParameterizedEvent
{
public:
	explicit ParameterizedEvent(NSVR_EventType);
	ParameterizedEvent(ParameterizedEvent&&);
	ParameterizedEvent(const ParameterizedEvent&);
	~ParameterizedEvent() {}
	template<class T>
	bool Set(const char* key, T value);
	bool SetFloat(const char* key, float value);
	bool SetInt(const char* key, int value);
	bool SetFloats(const char* key, float* values, unsigned int length);
	
	
	template<typename T>
	T Get(const char* key, T defaultValue) const;

	NSVR_EventType type() const;

private:
	NSVR_EventType m_type;
	std::vector<event_param> m_params;
	std::mutex m_propLock;

	event_param* findParam(const char* key);
	const event_param* findParam(const char* key) const;
	
	template<typename T>
	void updateOrAdd(const char* key, T val);
};

template<class T>
inline bool ParameterizedEvent::Set(const char * key, T value)
{
	std::lock_guard<std::mutex> guard(m_propLock);
	updateOrAdd<T>(key, value);
	return true;
}

template<typename T>
inline T ParameterizedEvent::Get(const char * key, T defaultValue) const
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
inline void ParameterizedEvent::updateOrAdd(const char * key, T val)
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
