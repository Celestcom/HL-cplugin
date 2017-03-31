#pragma once
#include <string>
#include <iostream>

#include "IoService.h"
#include "Wire\FlatbuffDecoder.h"
#include "ClientMessenger.h"
#include <boost\asio\deadline_timer.hpp>
#include "HapticsPlayer.h"
#include "ScheduledEvent.h"
#include "EventList.h"
#include "NSLoader.h"

#include "NSLoader_Internal.h"
#pragma pack(1)


class Engine
{
public:
	Engine();
	~Engine();
	bool Poll(NSVR_System_Status*);
	int PollStatus(NSVR_System_Status*);
	uint32_t GenHandle();

	bool EngineCommand(NSVR_EngineCommand command);

	void HandleCommand(unsigned int handle, NSVR_PlaybackCommand);

	void GetError(NSVR_ErrorInfo* errorInfo);
	int CreateEffect(uint32_t handle, void *data, unsigned int size);
	int CreateEffect(EventList* list, uint32_t handle);
	int  PollTracking(NSVR_TrackingUpdate* q);

	int PollLogs(NSVR_LogEntry* entry);
private:
	IoService m_ioService;
	NSVR_TrackingUpdate m_cachedTracking;
	NullSpace::Communication::SuitStatus _suitStatus;
	NSVR_TrackingUpdate _tracking;
	uint32_t _currentHandleId;
	std::string _currentError;
	std::unique_ptr<FlatbuffDecoder> _decoder;
	bool _isEnginePlaying;
	ClientMessenger m_messenger;

	//haptics playing 
	HapticsPlayer m_player;

	boost::asio::deadline_timer m_hapticsExecutionTimer;
	boost::posix_time::milliseconds m_hapticsExecutionInterval;
	void executeTimestep();

	ScheduledEvent m_hapticsTimestep;

};

