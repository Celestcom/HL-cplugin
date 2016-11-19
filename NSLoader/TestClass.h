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
	std::unique_ptr<DependencyResolver> _resolver;
	Wire _wire;
	NullSpace::Communication::SuitStatus _suitStatus;
	NullSpaceDLL::TrackingUpdate _tracking;
	uint32_t _currentHandleId;
	std::string _currentError;
	std::unique_ptr<FlatbuffDecoder> _decoder;
public:
	

	TestClass(LPSTR param);
	TestClass();
	~TestClass();
	bool Poll();
	int PollStatus();
	uint32_t GenHandle();
	void PollTracking(NullSpaceDLL::TrackingUpdate& q);

	
	bool LoadSequence(LPSTR param);
	bool CreateSequence(uint32_t handle, LPSTR param, uint32_t loc);
	bool CreateSequence(uint32_t handle, uint32_t* data, uint32_t size);

	bool LoadPattern(LPSTR param);
	bool CreatePattern(uint32_t handle, LPSTR param);
	bool SavePattern(LPSTR name, uint32_t* data, uint32_t size);

	bool LoadExperience(LPSTR param);
	bool CreateExperience(uint32_t handle, LPSTR param);

	int PlayEffect(Effect e, Location loc, float duration, float time, unsigned int priority);
	void SetTrackingEnabled(bool wantTracking);
	void HandleCommand(unsigned int handle, short c);

	char* GetError();
};

