#pragma once


#include "PlayableEvent.h"


class BufferedHaptic : public PlayableEvent {

public:
	BufferedHaptic(float time);
	float duration() const override;

private:

	std::vector<Validator> makeValidators() const override;


	void doSerialize(NullSpaceIPC::HighLevelEvent& event) const override;

	void doParse(const ParameterizedEvent&) override;


	bool isEqual(const PlayableEvent& other) const override;

	//in Hz, so 320 = 320Hz
	float m_frequency;
	std::vector<float> m_samples;
};