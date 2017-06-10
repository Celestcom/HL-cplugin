#pragma once
#include "NSLoader_fwds.h"
#include <boost/variant.hpp>
#include <boost/optional.hpp>

#define DO_VALIDATION

#ifdef DO_VALIDATION
#define VALIDATE_KEY(key) do { if (!validate(key)) { return false; } } while (0)
#else 
#define VALIDATE_KEY(key)
#endif

typedef boost::variant<float, int, std::vector<float>> EventValue;
struct property {
	property();
	property(const char* key, const EventValue&);
	const char* key;
	EventValue value;
};


class ParameterizedEvent
{
public:
	ParameterizedEvent(NSVR_EventType);
	~ParameterizedEvent() {}

	bool SetFloat(const char* key, float value);
	bool SetInt(const char* key, int value);
	bool SetFloats(const char* key, float* values, unsigned int length);
	
	
	template<typename T>
	T Get(const char* key, T defaultValue) const;

	NSVR_EventType type() const;

private:
	boost::optional<property&> find(const char* key);
	boost::optional<property> findByValue(const char* key) const;
	void update_or_add(const char* key, const EventValue& val);
	std::vector<property> m_properties;
	NSVR_EventType m_type;

	
	
};


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

template<typename T>
inline T ParameterizedEvent::Get(const char * key, T defaultValue) const
{
	if (auto prop = findByValue(key)) {
		if (T result = boost::get<T>(prop->value)) {
			return result;
		}
	//	EventValue v = prop->value;
	}
	
	return defaultValue;
	
}
