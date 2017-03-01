#pragma once
#include <string>
#include <iostream>

#include "IoService.h"
#include "Wire\FlatbuffDecoder.h"
#include "ClientMessenger.h"
#include <boost\asio\deadline_timer.hpp>
#include "HapticsPlayer.h"
#pragma pack(1)


class Engine
{
private:
	IoService m_ioService;

	NullSpace::Communication::SuitStatus _suitStatus;
	NSVR_InteropTrackingUpdate _tracking;
	uint32_t _currentHandleId;
	std::string _currentError;
	std::unique_ptr<FlatbuffDecoder> _decoder;
	bool _isEnginePlaying;
	ClientMessenger m_messenger;

	//haptics playing 
	HapticsPlayer m_player;

	boost::asio::deadline_timer m_hapticsExecutionTimer;
	boost::posix_time::milliseconds m_hapticsExecutionInterval;
	void scheduleTimestep();
	void executeTimestep(const boost::system::error_code& ec);

public:
	

	Engine();
	~Engine();
	bool Poll();
	int PollStatus();
	uint32_t GenHandle();

	bool EngineCommand(NSVR_EngineCommand command);
	
	void HandleCommand(unsigned int handle, NSVR_HandleCommand);

	char* GetError();
	int CreateEffect(uint32_t handle, void *data, unsigned int size);
};

