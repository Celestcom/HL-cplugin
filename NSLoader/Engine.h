#pragma once
#include <string>
#include "DependencyResolver.h"
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

	std::unique_ptr<NodeDependencyResolver> _resolver;
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
	bool InitializeFromFilesystem(LPSTR path);
	//void PollTracking(NullSpaceDLL::InteropTrackingUpdate& q);
	bool GetPlayingStatus();
	bool EngineCommand(short command);
	
	void SetTrackingEnabled(bool wantTracking);
	void HandleCommand(unsigned int handle, short c);

//	void CreateHaptic(unsigned int handle, void* data, unsigned int size);
	char* GetError();
	int CreateEffect(uint32_t handle, void *data, unsigned int size);
};

