#include "stdafx.h"
#include "PlaybackHandle.h"

#include "EventList.h"
#include "HLVR.h"
#include "Engine.h"

PlaybackHandle::PlaybackHandle() : handle(0), engine{ nullptr }
{

}

int PlaybackHandle::Pause()
{
	if (engine != nullptr) {
		engine->HandlePause(handle);
		return HLVR_Ok;
	}
	return HLVR_Error_Unknown;

}


int PlaybackHandle::Play()
{
	if (engine != nullptr) {
		engine->HandlePlay(handle);
		return HLVR_Ok;
	}
	return HLVR_Error_Unknown;

}

int PlaybackHandle::Reset()
{
	if (engine != nullptr) {
		engine->HandleReset(handle);
		return HLVR_Ok;
	}
	return HLVR_Error_Unknown;

}


int PlaybackHandle::GetHandleInfo(HLVR_EffectInfo* infoPtr)
{
	if (engine != nullptr) {
		return engine->GetHandleInfo(handle, infoPtr);
	}
	return HLVR_Error_Unknown;
}

