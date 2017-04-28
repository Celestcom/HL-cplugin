#pragma once
#include <stdint.h>
#include "NSLoader_fwds.h"
class EventList;
class Engine;
class PlaybackHandle
{
public:
	int Bind(EventList* eventListPtr);
	PlaybackHandle();
	int Command(NSVR_PlaybackCommand command);
	~PlaybackHandle();
private:
	Engine* m_engine;
	uint32_t m_handle;
};

