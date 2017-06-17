#include "stdafx.h"
#include "ParameterizedEvent.h"
#include "NSLoader.h"
#include "BasicHapticEvent.h"
#include "CurveEvent.h"

ParameterizedEvent::ParameterizedEvent(NSVR_EventType type): 
	m_type(type),
	m_properties()
{
	//average event key count is 5, so preemptively reserve that much
	//change if proves unnecessary
	m_properties.reserve(5);
}


ParameterizedEvent::ParameterizedEvent(ParameterizedEvent && other):
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
	updateOrAdd<float>(key, value);
	return true;
}

bool ParameterizedEvent::SetInt(const char * key, int value)
{
	std::lock_guard<std::mutex> guard(m_propLock);
	VALIDATE_KEY(key);
	updateOrAdd<int>(key, value);
	return true;
}

bool ParameterizedEvent::SetFloats(const char * key, float * values, unsigned int length)
{
	std::lock_guard<std::mutex> guard(m_propLock);
	VALIDATE_KEY(key);
	std::vector<float> vec;
	vec.reserve(length);
	memcpy_s(&vec[0], vec.size(), &values[0], length);
	updateOrAdd<std::vector<float>>(key, std::move(vec));
	return true;
}

NSVR_EventType ParameterizedEvent::type() const
{
	return m_type;
}

event_param* ParameterizedEvent::findParam(const char * key)
{
	return const_cast<event_param*>(
		static_cast<const ParameterizedEvent*>(this)->findParam(key)
	);
}

const event_param * ParameterizedEvent::findParam(const char * key) const
{
	for (std::size_t i = 0; i < m_properties.size(); i++) {
		if (strcmp(m_properties.at(i).key.c_str(), key) == 0) {
			return &m_properties.at(i);
		}
	}

	return nullptr;
}

event_param::event_param(): 
	key(), 
	value()
{
}

event_param::event_param(const char* key, EventValue val):
	key(key),
	value(val)
{
}

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