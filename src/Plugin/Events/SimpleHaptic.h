#pragma once
#include <stdint.h>
#include "PlayableEvent2.h"


struct SimpleHaptic : public PlayableEvent2 {
	float duration;
	uint32_t waveform;
	float strength;

};