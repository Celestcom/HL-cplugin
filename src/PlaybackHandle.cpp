#include "stdafx.h"
#include "PlaybackHandle.h"
#include "Engine.h"

#include "EventList.h"
#include "NSLoader.h"
int PlaybackHandle::Bind(EventList * eventListPtr)
{
	assert(eventListPtr != nullptr);
	if (m_engine != nullptr) {
		m_engine->HandleCommand(m_handle, NSVR_PlaybackCommand(3));
	}
	m_engine = eventListPtr->EnginePtr();
	assert(m_engine != nullptr);
	m_handle = m_engine->GenHandle();

	m_engine->CreateEffect(eventListPtr, m_handle);
	return NSVR_Success_Unqualified;
}

PlaybackHandle::PlaybackHandle(): m_engine(nullptr)
{
}

int PlaybackHandle::Command(NSVR_PlaybackCommand command)
{
	if (m_engine != nullptr) {
		m_engine->HandleCommand(m_handle, command);
		return NSVR_Success_Unqualified;
	}
	else {
		return -1;
	}
}


PlaybackHandle::~PlaybackHandle()
{
	//todo: pay back the debt
	//this is bad
	//can throw exceptions
	//need to rearchitect playback handles
	if (m_engine != nullptr) {
		m_engine->HandleCommand(m_handle, NSVR_PlaybackCommand(3));
	}

}

int PlaybackHandle::GetHandleInfo(NSVR_HandleInfo* infoPtr)
{
	//todo: pay back the debt
	//this is bad
	//can throw exceptions
	//need to rearchitect playback handles
	if (m_engine != nullptr) {
		return m_engine->GetHandleInfo(m_handle, infoPtr);
	}

	return NSVR_Error_NullArgument;
}

