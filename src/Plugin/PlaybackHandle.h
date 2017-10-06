#pragma once
#include <stdint.h>
#include "HLVR_Forwards.h"



class Engine;
class PlaybackHandle
{
public:
	PlaybackHandle();
	PlaybackHandle(const PlaybackHandle&) = delete;

	int Pause();
	int Play();
	int Reset();
	int GetHandleInfo(HLVR_EffectInfo* infoPtr);

	uint32_t handle;
	Engine* engine;

};

