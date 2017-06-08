#pragma once
#include <boost/variant.hpp>
#include "ParameterizedEvent.h"
#include "PlayableEvent.h"
#include "NSLoader.h"

class BasicHapticEvent : public ParameterizedEvent, public PlayableEvent {
public:	
	BasicHapticEvent();

	/* ParameterizedEvent implementation */
	bool doSetFloat(const char* key, float value) override;
	bool doSetInt(const char* key, int value) override;
	BasicHapticEvent* doClone() override;
	NSVR_EventType type() const override;
	static constexpr NSVR_EventType descriptor = NSVR_EventType::NSVR_EventType_BasicHapticEvent;

	/* PlayableEvent implementation*/
	uint32_t area() const override;
	float time() const override;
	float duration() const override;

	float strength() const;
	uint32_t effectFamily() const;
private:
	float m_time;
	float m_strength;
	float m_duration;
	uint32_t m_area;
	std::string m_parsedEffectFamily;
	uint32_t m_requestedEffectFamily;
};


typedef boost::variant<BasicHapticEvent> SuitEvent;
