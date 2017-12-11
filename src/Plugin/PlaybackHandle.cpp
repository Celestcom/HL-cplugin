#include "stdafx.h"
#include "PlaybackHandle.h"

#include "EventList.h"
#include "HLVR.h"
#include "Engine.h"

PlaybackHandle::PlaybackHandle() : handle(0), engine{ nullptr }
{

}

bool PlaybackHandle::IsBound() const
{
	return handle != 0;
}

int PlaybackHandle::Pause()
{
	if (engine != nullptr) {
		return engine->HandlePause(handle);
	}
	return HLVR_Error_EmptyHandle;
}


int PlaybackHandle::Play()
{
	if (engine != nullptr) {
		return engine->HandlePlay(handle);
	}
	return HLVR_Error_EmptyHandle;

}

int PlaybackHandle::Reset()
{
	if (engine != nullptr) {
		return engine->HandleReset(handle);
	}
	return HLVR_Error_EmptyHandle;

}


int PlaybackHandle::GetInfo(HLVR_EffectInfo* infoPtr) const
{
	if (engine != nullptr) {
		return engine->GetInfo(handle, infoPtr);
	}
	return HLVR_Error_EmptyHandle;
}

void PlaybackHandle::bind(uint32_t handle, Engine * engine)
{
	this->handle = handle;
	this->engine = engine;
}

