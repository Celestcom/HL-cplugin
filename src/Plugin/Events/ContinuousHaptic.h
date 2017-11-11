#pragma once

#include "PlayableEvent.h"


class ContinuousHaptic : public PlayableEvent {

public:
	ContinuousHaptic(float time);
	virtual float duration() const override;

private:

	virtual std::vector<Validator> makeValidators() const override;


	virtual void doSerialize(NullSpaceIPC::HighLevelEvent& event) const override;

	virtual void doParse(const ParameterizedEvent&) override;


	virtual bool isEqual(const PlayableEvent& other) const override;


	float m_strength;
};