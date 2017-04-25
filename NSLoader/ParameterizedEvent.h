#pragma once


class ParameterizedEvent
{
public:
	ParameterizedEvent();
	virtual ~ParameterizedEvent() {}
	bool SetFloat(const char* key, float value);
	bool SetInt(const char* key, int value);
private:
	virtual bool doSetFloat(const char* key, float value) = 0;
	virtual bool doSetInt(const char* key, int value) = 0;
};

