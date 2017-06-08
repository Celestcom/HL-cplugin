#include "stdafx.h"
#include "ParameterizedEvent.h"
#include "NSLoader.h"
#include "BasicHapticEvent.h"
#include "CurveEvent.h"
//Check that the string is null terminated and less than 32 characters. Not sure if this is appropriate.
//Still evaluating

//#define DO_VALIDATION

#ifdef DO_VALIDATION
#define VALIDATE_KEY(key) do { if (!validate(key)) { return false; } } while (0)
#else 
#define VALIDATE_KEY(key)
#endif


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

ParameterizedEvent::ParameterizedEvent()
{
}


bool ParameterizedEvent::SetFloat(const char * key, float value)
{
	VALIDATE_KEY(key);
	return doSetFloat(key, value);
}

bool ParameterizedEvent::SetInt(const char * key, int value)
{
	VALIDATE_KEY(key);
	return doSetInt(key, value);
}

bool ParameterizedEvent::SetFloats(const char * key, float * values, unsigned int length)
{
	VALIDATE_KEY(key);
	return doSetFloats(key, values, length);
}

ParameterizedEvent* ParameterizedEvent::makeEvent(NSVR_EventType type)
{
	switch (type) {
	case NSVR_EventType::NSVR_EventType_BasicHapticEvent:
		return new BasicHapticEvent();
	case NSVR_EventType::NSVR_EventType_CurveHapticEvent:
		return new CurveEvent();
	default:
		return nullptr;
	}
}

std::unique_ptr<ParameterizedEvent> ParameterizedEvent::Clone()
{
	return std::unique_ptr<ParameterizedEvent>(doClone());
}


