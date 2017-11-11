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
	
private:
	std::vector<Validator> makeValidators() const override;

	void doParse(const ParameterizedEvent&) override;
	void doSerialize(NullSpaceIPC::HighLevelEvent& event) const override;

	float m_strength;
	float m_duration;
	std::string m_parsedEffectFamily;
	uint32_t m_requestedEffectFamily;



	virtual bool isEqual(const PlayableEvent& other) const override;


};

