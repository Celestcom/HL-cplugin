#include "stdafx.h"
#include "PlaybackHandle.h"

#include "EventList.h"
#include "NSLoader.h"
#include "Engine.h"

PlaybackHandle::PlaybackHandle() : handle(0), engine{ nullptr }
{

}

PlaybackHandle::~PlaybackHandle()
{
	engine = nullptr;
	handle = 0;
}

int PlaybackHandle::Command(NSVR_PlaybackCommand command)
{
	if (engine != nullptr) {
		engine->HandleCommand(handle, command);
		return NSVR_Success_Unqualified;
	}
	
	
	return NSVR_Error_Unknown;
	
}




int PlaybackHandle::GetHandleInfo(NSVR_HandleInfo* infoPtr)
{
	//todo: pay back the debt
	//this is bad
	//can throw exceptions
	//need to rearchitect playback handles
	if (engine != nullptr) {
		return engine->GetHandleInfo(handle, infoPtr);
	}
	


	return NSVR_Error_Unknown;
}

