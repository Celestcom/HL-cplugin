#pragma once
#include <boost/variant.hpp>
#include "ParameterizedEvent.h"
#include "PlayableEvent.h"
#include "NSLoader.h"

class BasicHapticEvent : public PlayableEvent {
public:	
	BasicHapticEvent();

	float strength() const;
	uint32_t effectFamily() const;

	/* PlayableEvent impl */
	uint32_t area() const override;
	float time() const override;
	float duration() const override;
	NSVR_EventType type() const override;
	bool parse(const ParameterizedEvent&) override;
	static constexpr NSVR_EventType descriptor = NSVR_EventType::NSVR_EventType_BasicHapticEvent;

private:
	float m_time;
	float m_strength;
	float m_duration;
	uint32_t m_area;
	std::string m_parsedEffectFamily;
	uint32_t m_requestedEffectFamily;



	virtual bool isEqual(const PlayableEvent& other) const override;

};


typedef boost::variant<BasicHapticEvent> SuitEvent;
