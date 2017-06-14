#include "stdafx.h"
#include "ParameterizedEvent.h"
#include "NSLoader.h"
#include "BasicHapticEvent.h"
#include "CurveEvent.h"
//Check that the string is null terminated and less than 32 characters. Not sure if this is appropriate.
//Still evaluating



bool validate(const char* key) {
	const unsigned int max_key_len = 32;
	if (key[0] == 0) { //no empty string
		return false;
	}
	for (int i = 0; i < max_key_len; i++) {
		if (key[i] == 0) //if null terminator 
		{
			return true;	
		}else if (!isalnum(key[i])) {
			return false;
		}
	}

	return true;
}

ParameterizedEvent::ParameterizedEvent(NSVR_EventType t): m_properties(), m_type(t)
{
}


ParameterizedEvent::ParameterizedEvent(ParameterizedEvent && other) : 
	m_type(other.m_type),
	m_properties(std::move(other.m_properties))
{
	
}

ParameterizedEvent::ParameterizedEvent(const ParameterizedEvent & other):
	m_type(other.m_type),
	m_properties(other.m_properties)
{
}

bool ParameterizedEvent::SetFloat(const char * key, float value)
{
	VALIDATE_KEY(key);
	update_or_add(key, value);
	return true;
}

bool ParameterizedEvent::SetInt(const char * key, int value)
{
	VALIDATE_KEY(key);
	update_or_add(key, value);
	return true;
}

bool ParameterizedEvent::SetFloats(const char * key, float * values, unsigned int length)
{
	VALIDATE_KEY(key);
	std::vector<float> vec(length);
	memcpy_s(&vec[0], vec.size(), &values[0], length);
	update_or_add(key, std::move(vec));
	return true;
}

NSVR_EventType ParameterizedEvent::type() const
{
	return m_type;
}

//this reference will become invalid if anybody changes the vector
boost::optional<property&> ParameterizedEvent::find(const char * key) 
{
	auto it = std::find_if(m_properties.begin(), m_properties.end(), [&key](auto& prop) {
		return strcmp(key, prop.key) == 0;
	});

	if (it != m_properties.end()) {
		return *it;
	}

	return boost::none;
}

boost::optional<property> ParameterizedEvent::findByValue(const char * key) const
{
	auto it = std::find_if(m_properties.begin(), m_properties.end(), [&key](auto& prop) {
		return strcmp(key, prop.key) == 0;
	});

	if (it != m_properties.end()) {
		return *it;
	}

	return boost::none;
}


void ParameterizedEvent::update_or_add(const char * key, const EventValue & val)
{
	if (auto existing = find(key)) {
		existing->value = val;
	}
	else {
		m_properties.emplace_back(key, val);
	}
}



property::property() : key(nullptr), value()
{

}

property::property(const char* key, const EventValue& val):
	key(key),
	value(val)
{

}
