#pragma once

#include "PlayableEvent.h"

class BeginAnalogAudio : public PlayableEvent {
public:
	BeginAnalogAudio(float time);
	float duration() const override { return 0.0f; }
private:
	void doSerialize(NullSpaceIPC::HighLevelEvent& event) const override;
	void doParse(const ParameterizedEvent& event) override;
	bool isEqual(const PlayableEvent& other) const override { return true; }
};


class EndAnalogAudio : public PlayableEvent {
public:
	EndAnalogAudio(float time);
	float duration() const override { return 0.0f; }
private:
	void doSerialize(NullSpaceIPC::HighLevelEvent& event) const override;
	void doParse(const ParameterizedEvent&) override;
	bool isEqual(const PlayableEvent& other) const override { return true; }

};