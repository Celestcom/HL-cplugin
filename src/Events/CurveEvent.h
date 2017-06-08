#pragma once
#include <boost/variant.hpp>
#include "ParameterizedEvent.h"
#include "PlayableEvent.h"
#include "NSLoader.h"


class CurveEvent : public ParameterizedEvent, public PlayableEvent {
public:
	CurveEvent();


	/* ParameterizedEvent implementation */
	bool doSetFloat(const char* key, float value) override;
	bool doSetFloats(const char* key, float* values, unsigned int length) override;
	bool doSetInt(const char* key, int value) override;
	CurveEvent* doClone() override;
	NSVR_EventType type() const override;
	static constexpr NSVR_EventType descriptor = NSVR_EventType::NSVR_EventType_CurveHapticEvent;


	/* PlayableEvent implementation */
	virtual float duration() const override;
	virtual float time() const override;
	virtual uint32_t area() const override;

private:
	float m_time;
	uint32_t m_area;
	float m_duration;
	std::vector<float> m_volumes;
	std::vector<float> m_timePoints;
	


};


