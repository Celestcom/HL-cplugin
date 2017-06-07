#pragma once
#include <stdint.h>
#include "NSLoader_fwds.h"



class Engine;
class PlaybackHandle
{
public:
	PlaybackHandle();
	PlaybackHandle(const PlaybackHandle&) = delete;


	int Command(NSVR_PlaybackCommand command);
	int GetHandleInfo(NSVR_HandleInfo* infoPtr);

	uint32_t handle;
	Engine* engine;

};

