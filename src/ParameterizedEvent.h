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
