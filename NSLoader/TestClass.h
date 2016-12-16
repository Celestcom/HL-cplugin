#pragma once
#include <string>
#include "DependencyResolver.h"
#include <iostream>
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include "Wire\Wire.h"
#include "Wire\EncodingOperations.h"
#include "Wire\FlatbuffDecoder.h"
#pragma pack(1)


class TestClass
{
private:
	std::unique_ptr<NodeDependencyResolver> _resolver;
	Wire _wire;
	NullSpace::Communication::SuitStatus _suitStatus;
	NullSpaceDLL::InteropTrackingUpdate _tracking;
	uint32_t _currentHandleId;
	std::string _currentError;
	std::unique_ptr<FlatbuffDecoder> _decoder;
	bool _isEnginePlaying;
public:
	

	TestClass();
	~TestClass();
	bool Poll();
	int PollStatus();
	uint32_t GenHandle();
	bool InitializeFromFilesystem(LPSTR path);
	void PollTracking(NullSpaceDLL::InteropTrackingUpdate& q);
	bool GetPlayingStatus();
	bool Load(LPSTR param, int filetype);
	bool EngineCommand(short command);
	bool LoadSequence(LPSTR param);
	bool CreateSequence(uint32_t handle, LPSTR param, uint32_t loc);

	bool LoadPattern(LPSTR param);
	bool CreatePattern(uint32_t handle, LPSTR param);

	bool LoadExperience(LPSTR param);
	bool CreateExperience(uint32_t handle, LPSTR param);

	void SetTrackingEnabled(bool wantTracking);
	void HandleCommand(unsigned int handle, short c);

	void CreateHaptic(unsigned int handle, void* data, unsigned int size);
	char* GetError();
};

