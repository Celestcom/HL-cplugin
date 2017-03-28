#include "stdafx.h"
#include "ParameterizedEvent.h"



bool validate(const char* key) {
	const unsigned int max_key_len = 32;
	if (key[0] == 0) { //no empty string
		return false;
	}
	for (int i = 0; i < max_key_len; i++) {
		if (key[i] == 0) //if null terminator 
		{
			return true;	
		}

		if (!isalnum(key[i])) {
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
	if (validate(key)) {
		return doSetFloat(key, value);
	}
	else { return false; }
}

bool ParameterizedEvent::SetInt(const char * key, int value)
{
	if (validate(key)) {
		return doSetInt(key, value);
	}
	else { return false; }
}



