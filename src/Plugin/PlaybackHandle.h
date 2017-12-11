#pragma once
#include <stdint.h>
#include "HLVR_Forwards.h"



class Engine;
class PlaybackHandle
{
public:
	PlaybackHandle();
	PlaybackHandle(const PlaybackHandle&) = delete;

	bool IsBound() const;
	int Pause();
	int Play();
	int Reset();
	int GetInfo(HLVR_EffectInfo* infoPtr) const;

	void bind(uint32_t handle, Engine* engine);

	uint32_t handle;
	Engine* engine;

};

