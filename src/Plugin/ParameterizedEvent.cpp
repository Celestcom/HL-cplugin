#include "stdafx.h"
#include "ParameterizedEvent.h"
#include "NSLoader.h"
#include "BasicHapticEvent.h"
#include "CurveEvent.h"

ParameterizedEvent::ParameterizedEvent(NSVR_EventType type): 
	m_type(type),
	m_params()
{
	//average event key count is 5, so preemptively reserve that much
	//change if proves unnecessary
	m_params.reserve(5);
}


ParameterizedEvent::ParameterizedEvent(ParameterizedEvent && other):
	m_type(other.m_type),
	m_params(std::move(other.m_params))
{
}

ParameterizedEvent::ParameterizedEvent(const ParameterizedEvent & other) :
	m_type(other.m_type),
	m_params(other.m_params)
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
	std::vector<float> vec(values, values + length);
	updateOrAdd<std::vector<float>>(key, std::move(vec));
	return true;
}

bool ParameterizedEvent::SetUInt32s(const char * key, uint32_t * values, unsigned int length)
{
	std::lock_guard<std::mutex> guard(m_propLock);
	VALIDATE_KEY(key);
	std::vector<uint32_t> vec(values, values + length);
	updateOrAdd<std::vector<uint32_t>>(key, std::move(vec));
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
	for (const auto& param : m_params) {
		if (strcmp(param.key.c_str(), key) == 0) {
			return &param;
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