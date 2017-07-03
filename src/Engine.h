#pragma once
#include <string>
#include <iostream>

#include "IoService.h"
#include "ClientMessenger.h"
#include <boost\asio\deadline_timer.hpp>
#include "HapticsPlayer.h"
#include "ScheduledEvent.h"
#include "EventList.h"
#include "NSLoader.h"
#include "MyTestLog.h"
#include "IHapticDevice.h"
#include "NSLoader_Internal.h"
#include "EngineIsAlive.h"
#include "EngineCommand.h"

class Engine
{
public:
	Engine();


	~Engine();
	int PollStatus(NSVR_ServiceInfo*);
	uint32_t GenerateHandle();
	int PollDevice(NSVR_DeviceInfo *);
	bool DoEngineCommand(::EngineCommand command);
	int GetEngineStats(NSVR_SystemStats* stats);
	void HandleCommand(unsigned int handle, NSVR_PlaybackCommand);
	void ReleaseHandle(unsigned int handle);
	int CreateEffect(EventList * list, HapticHandle * handle);
	int  PollTracking(NSVR_TrackingUpdate* q);

	int PollLogs(NSVR_LogEntry* entry);

	int EnableAudio(NSVR_AudioOptions* options);
	int DisableAudio();

	int SubmitRawCommand(uint8_t* buffer, int length);

	int Sample(uint16_t* strengths, uint32_t* areas, uint32_t* families, int length, unsigned int* resultCount);

	int DumpDeviceDiagnostics();
	int SetStrengths(uint16_t* strengths, uint32_t* areas, unsigned int length);
	int GetHandleInfo(uint32_t m_handle, NSVR_EffectInfo* infoPtr);

private:
	IoService m_ioService;
	NSVR_TrackingUpdate m_cachedTrackingUpdate;
	uint32_t m_currentHandleId;
	bool m_isHapticsSystemPlaying;
	ClientMessenger m_messenger;
	EventRegistry m_registry;

	std::unique_ptr<IHapticDevice> m_hardlightSuit;
	HapticsPlayer m_player;

	boost::posix_time::milliseconds m_hapticsExecutionInterval;
	void executeTimestep();

	ScheduledEvent m_hapticsTimestep;

	boost::shared_ptr<MyTestLog> m_log;



	void setupUserFacingLogSink();

	void setupFileLogSink();
};

