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
#include "NSLoader_Internal.h"
#pragma pack(1)

typedef enum NSVR_EngineCommand_ {
	NSVR_EngineCommand_ResumeAll = 1,
	NSVR_EngineCommand_PauseAll,
	NSVR_EngineCommand_DestroyAll,
	NSVR_EngineCommand_EnableTracking,
	NSVR_EngineCommand_DisableTracking
} NSVR_EngineCommand;

class Engine
{
public:
	Engine();
	~Engine();
	int PollStatus(NSVR_ServiceInfo*);
	uint32_t GenHandle();
	int PollDevice(NSVR_DeviceInfo *);
	bool EngineCommand(NSVR_EngineCommand command);
	int GetEngineStats(NSVR_SystemStats* stats);
	void HandleCommand(unsigned int handle, NSVR_PlaybackCommand);

	void GetError(NSVR_ErrorInfo* errorInfo);
	int CreateEffect(EventList* list, uint32_t handle);
	int  PollTracking(NSVR_TrackingUpdate* q);

	int PollLogs(NSVR_LogEntry* entry);

	int EnableAudio(NSVR_AudioOptions* options);
	int DisableAudio();

	int SubmitRawCommand(uint8_t* buffer, int length);

	int Sample(uint16_t* strengths, uint32_t* areas, uint32_t* families, int length, unsigned int* resultCount);

	int DumpDeviceDiagnostics();
	int SetStrengths(uint16_t* strengths, uint32_t* areas, int length);

private:
	IoService m_ioService;
	NSVR_TrackingUpdate m_cachedTracking;
	NSVR_TrackingUpdate _tracking;
	uint32_t _currentHandleId;
	std::string _currentError;
	bool _isEnginePlaying;
	ClientMessenger m_messenger;
	EventRegistry m_registry;

	HapticsPlayer m_player;

	boost::asio::deadline_timer m_hapticsExecutionTimer;
	boost::posix_time::milliseconds m_hapticsExecutionInterval;
	void executeTimestep();

	ScheduledEvent m_hapticsTimestep;

	boost::shared_ptr<MyTestLog> m_log;



	
};

