#pragma once
#include <boost/variant.hpp>
#include "ParameterizedEvent.h"
#include "PlayableEvent.h"
#include "HLVR.h"


class DiscreteHapticEvent : public PlayableEvent {
public:	
	DiscreteHapticEvent(float time);
	


	float strength() const;
	uint32_t effectFamily() const;

	/* PlayableEvent impl */
	float duration() const override;
	bool doParse(const ParameterizedEvent&) override;
	std::vector<Validator> makeValidators() const override;
	void doSerialize(NullSpaceIPC::HighLevelEvent& event) const override;

private:
	static constexpr HLVR_EventType descriptor = HLVR_EventType::HLVR_EventType_DiscreteHaptic;

	float m_strength;
	float m_duration;
	std::string m_parsedEffectFamily;
	uint32_t m_requestedEffectFamily;



	virtual bool isEqual(const PlayableEvent& other) const override;


};

