#include "stdafx.h"
#include "ParameterizedEvent.h"


//Check that the string is null terminated and less than 32 characters. Not sure if this is appropriate.
//Still evaluating
//#define DO_VALIDATION

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
#ifdef DO_VALIDATION
	if (!validate(key)) {
		return false;
	}
#endif
	return doSetFloat(key, value);
}

bool ParameterizedEvent::SetInt(const char * key, int value)
{
#ifdef DO_VALIDATION
	if (!validate(key)) {
		return false;
	}
#endif
	return doSetInt(key, value);
}



