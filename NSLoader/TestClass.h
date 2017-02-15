#pragma once
#include <string>
#include "DependencyResolver.h"
#include <iostream>

#include "IoService.h"
#include "Wire\FlatbuffDecoder.h"
#include "ClientMessenger.h"
#pragma pack(1)


class TestClass
{
private:
	IoService m_ioService;

	std::unique_ptr<NodeDependencyResolver> _resolver;
	NullSpace::Communication::SuitStatus _suitStatus;
	NullSpaceDLL::InteropTrackingUpdate _tracking;
	uint32_t _currentHandleId;
	std::string _currentError;
	std::unique_ptr<FlatbuffDecoder> _decoder;
	bool _isEnginePlaying;
	ClientMessenger m_messenger;


public:
	

	TestClass();
	~TestClass();
	bool Poll();
	int PollStatus();
	uint32_t GenHandle();
	bool InitializeFromFilesystem(LPSTR path);
	//void PollTracking(NullSpaceDLL::InteropTrackingUpdate& q);
	bool GetPlayingStatus();
	bool EngineCommand(short command);
	
	void SetTrackingEnabled(bool wantTracking);
	void HandleCommand(unsigned int handle, short c);

	void CreateHaptic(unsigned int handle, void* data, unsigned int size);
	char* GetError();
};

