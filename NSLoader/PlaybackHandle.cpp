#include "stdafx.h"
#include "PlaybackHandle.h"
#include "Engine.h"

#include "EventList.h"
#include "NSLoader.h"
int PlaybackHandle::Bind(EventList * eventListPtr)
{
	//todo: This should do sorting automatically by time
	if (m_engine != nullptr) {
		m_engine->HandleCommand(m_handle, NSVR_PlaybackCommand(3));
	}
	m_engine = eventListPtr->EnginePtr();
	m_handle = m_engine->GenHandle();

	m_engine->CreateEffect(eventListPtr, m_handle);
	return NSVR_Success_Unqualified;
}

PlaybackHandle::PlaybackHandle()
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
	if (m_engine != nullptr) {
		m_engine->HandleCommand(m_handle, NSVR_PlaybackCommand(3));
	}
}
