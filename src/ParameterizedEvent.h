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
struct event_attribute {
	event_attribute();
	event_attribute(const char* key, EventValue);
	const char* key;
	EventValue value;
};


class ParameterizedEvent
{
public:
	ParameterizedEvent(NSVR_EventType);
	~ParameterizedEvent() {}
	ParameterizedEvent(ParameterizedEvent&&);
	ParameterizedEvent(const ParameterizedEvent&);
	bool SetFloat(const char* key, float value);
	bool SetInt(const char* key, int value);
	bool SetFloats(const char* key, float* values, unsigned int length);
	
	
	template<typename T>
	T Get(const char* key, T defaultValue) const;

	NSVR_EventType type() const;

private:
	event_attribute* findMyCoolThing(const char* key);
	boost::optional<event_attribute> findByValue(const char* key) const;
	void update_or_add(const char* key, EventValue val);

	template<typename T>
	void update_or_add(const char* key, T val);
	std::vector<event_attribute> m_properties;
	NSVR_EventType m_type;
	std::mutex m_propLock;
	
	
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
	try {
		if (auto prop = findByValue(key)) {
			if (T result = boost::get<T>(prop->value)) {
				return result;
			}
			//	EventValue v = prop->value;
		}
	}
	catch (const boost::bad_get&) {
		BOOST_LOG_TRIVIAL(error) << std::this_thread::get_id() <<
			"[ParameterizedEvent " << this << "] Bad get! key = " << key << " T = " << typeid(T).name();
		for (const auto& prop : m_properties) {
			BOOST_LOG_TRIVIAL(error) << std::this_thread::get_id() <<
				"\t " << prop.key << ": type = " << prop.value.which();
		}
	}
	return defaultValue;
	
}

template<typename T>
inline void ParameterizedEvent::update_or_add(const char * key, T val)
{
	BOOST_LOG_TRIVIAL(error) << std::this_thread::get_id() <<
		"[ParameterizedEvent] Setting key " << key << " to type T = " << typeid(T).name();
	event_attribute* existing = findMyCoolThing(key);
	if (existing != nullptr) {
		existing->value = val;
	}
	else {
		m_properties.emplace_back(key, val);
	}
}
