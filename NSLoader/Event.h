#pragma once
#include <boost\variant.hpp>

#include "NSLoader.h"
#include "SuitEvent.h"
class Event
{
public:
	Event(NSVR_EventType eventType);
	~Event();
	bool SetFloat(const char* key, float f);
	bool SetInteger(const char* key, int i);
	boost::variant<BasicHapticEvent> GetEvent();
private:
	boost::variant<BasicHapticEvent> m_event;
	
	template<class T>
	class SetParamVisitor : public boost::static_visitor<bool>
	{
	public:
		SetParamVisitor(const char* key, T f);
		bool operator()(BasicHapticEvent& h);
	private:
		const char* m_key;
		T m_value;
	};
};

template<class T>
inline Event::SetParamVisitor<T>::SetParamVisitor(const char * key, T f):m_key(key), m_value(f)
{
}

template<>
inline bool Event::SetParamVisitor<float>::operator()(BasicHapticEvent & h)
{
	if ("strength" == m_key) {
		h.Strength = m_value;
		return true;
	}
	else if ("time" == m_key) {
		h.Time = m_value;
		return true;
	}
	else if ("duration" == m_key) {
		h.Duration = m_value;
		return true;
	}
	else {
		return false;
	}
}


template<>
inline bool Event::SetParamVisitor<int>::operator()(BasicHapticEvent & h)
{
	if ("area" == m_key) {
		h.Area = m_value;
		return true;
	}
	else {
		return false;
	}
}
