#include "stdafx.h"
#include "ParameterizedEvent.h"
#include "NSLoader.h"
#include "BasicHapticEvent.h"

ParameterizedEvent::ParameterizedEvent(NSVR_EventType type): 
	m_type(type),
	m_params()
{
	//average event key count is 5, so preemptively reserve that much
	//change if proves unnecessary
	m_params.reserve(5);
}


//ParameterizedEvent::ParameterizedEvent(ParameterizedEvent && other):
//	m_type(other.m_type),
//	m_params(std::move(other.m_params))
//{
//}

//ParameterizedEvent::ParameterizedEvent(const ParameterizedEvent & other) :
//	m_type(other.m_type),
//	m_params(other.m_params)
//{
//}

//bool ParameterizedEvent::SetFloat(NSVR_EventKey key, float value)
//{
////	std::lock_guard<std::mutex> guard(m_propLock);
//	updateOrAdd<float>(key, value);
//	return true;
//}
//
//bool ParameterizedEvent::SetInt(NSVR_EventKey key, int value)
//{
////	std::lock_guard<std::mutex> guard(m_propLock);
//	updateOrAdd<int>(key, value);
//	return true;
//}
//
//bool ParameterizedEvent::SetFloats(NSVR_EventKey key, float * values, unsigned int length)
//{
////	std::lock_guard<std::mutex> guard(m_propLock);
//	std::vector<float> vec(values, values + length);
//	updateOrAdd<std::vector<float>>(key, std::move(vec));
//	return true;
//}
//
//bool ParameterizedEvent::SetUInt32s(NSVR_EventKey key, uint32_t * values, unsigned int length)
//{
//	std::vector<uint32_t> vec(values, values + length);
//	updateOrAdd<std::vector<uint32_t>>(key, std::move(vec));
//	return true;
//}
//
//bool ParameterizedEvent::SetUInt64s(NSVR_EventKey key, uint64_t * values, unsigned int length)
//{
//	std::vector<uint64_t> vec(values, values + length);
//	updateOrAdd<std::vector<uint64_t>>(key, std::move(vec));
//	return true;
//
//
//}

NSVR_EventType ParameterizedEvent::type() const
{
	return m_type;
}

event_param* ParameterizedEvent::findParam(NSVR_EventKey key)
{
	return const_cast<event_param*>(
		static_cast<const ParameterizedEvent*>(this)->findParam(key)
	);
}

const event_param * ParameterizedEvent::findParam(NSVR_EventKey key) const
{
	for (const auto& param : m_params) {
		if (param.key == key) {
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

event_param::event_param(NSVR_EventKey key, EventValue val):
	key(key),
	value(val)
{
}
