#include "stdafx.h"
#include "ParameterizedEvent.h"
#include "NSLoader.h"
#include "BasicHapticEvent.h"
#include "CurveEvent.h"
//Check that the string is null terminated and less than 32 characters. Not sure if this is appropriate.
//Still evaluating



bool validate(const char* key) {
	const std::size_t max_key_len = 32;
	if (key == nullptr || key[0] == '\0') { 
		return false;
	}

	for (std::size_t i = 1; i < max_key_len; i++) {
		if (key[i] == '\0') {
			return true;
		}
	}

	return false;
}

ParameterizedEvent::ParameterizedEvent(NSVR_EventType t) : m_properties(), m_type(t)
{
	m_properties.reserve(5);
}


ParameterizedEvent::ParameterizedEvent(ParameterizedEvent && other) :
	m_type(other.m_type),
	m_properties(std::move(other.m_properties))
{

}

ParameterizedEvent::ParameterizedEvent(const ParameterizedEvent & other) :
	m_type(other.m_type),
	m_properties(other.m_properties)
{
}

bool ParameterizedEvent::SetFloat(const char * key, float value)
{
	std::lock_guard<std::mutex> guard(m_propLock);
	VALIDATE_KEY(key);
	update_or_add<float>(key, value);
	return true;
}

bool ParameterizedEvent::SetInt(const char * key, int value)
{
	std::lock_guard<std::mutex> guard(m_propLock);
	VALIDATE_KEY(key);
	update_or_add<int>(key, value);
	return true;
}

bool ParameterizedEvent::SetFloats(const char * key, float * values, unsigned int length)
{
	std::lock_guard<std::mutex> guard(m_propLock);

	VALIDATE_KEY(key);
	std::vector<float> vec(length);
	memcpy_s(&vec[0], vec.size(), &values[0], length);
	update_or_add<std::vector<float>>(key, std::move(vec));
	return true;
}

NSVR_EventType ParameterizedEvent::type() const
{
	return m_type;
}

//this reference will become invalid if anybody changes the vector
event_attribute* ParameterizedEvent::findAttribute(const char * key)
{
	for (std::size_t i = 0; i < m_properties.size(); i++) {
		if (strcmp(m_properties.at(i).key.c_str(), key) == 0	) {
			return &m_properties.at(i);
		}
	}
	
	return nullptr;
	

}

const event_attribute * ParameterizedEvent::findAttribute(const char * key) const
{
	for (std::size_t i = 0; i < m_properties.size(); i++) {
		if (strcmp(m_properties.at(i).key.c_str(), key) == 0) {
			return &m_properties.at(i);
		}
	}

	return nullptr;
}




event_attribute::event_attribute() : key(), value()
{

}

event_attribute::event_attribute(const char* key, EventValue val):
	key(key),
	value(val)
{

}
