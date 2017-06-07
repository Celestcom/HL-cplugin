#pragma once


class ParameterizedEvent
{
public:
	ParameterizedEvent();
	virtual ~ParameterizedEvent() {}
	bool SetFloat(const char* key, float value);
	bool SetInt(const char* key, int value);
	bool SetFloats(const char* key, float* values, unsigned int length);

private:
	virtual bool doSetFloat(const char* key, float value) { return false; }
	virtual bool doSetInt(const char* key, int value) { return false; }
	virtual bool doSetFloats(const char* key, float* values, unsigned int length) { return false; }
};

template<class T>
inline T minimum_bound(T minimum, T value) {
	return std::max<T>(minimum, value);
}

template<class T>
inline T maximum_bound(T max, T value) {
	return std::min<T>(max, value);
}

template<class T>
inline T constrain(T min, T val, T max) {
	return std::clamp<T>(val, min, max);
}
