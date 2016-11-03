#pragma once
#include <string>
#include "DependencyResolver.h"
#include <iostream>
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include "Wire\Wire.h"
#include "Wire\EncodingOperations.h"
#pragma pack(1)


class TestClass
{
private:
	DependencyResolver _resolver;
	Wire _wire;
	NullSpace::Communication::SuitStatus _suitStatus;
	NullSpaceDLL::TrackingUpdate _tracking;
public:
	TestClass(LPSTR param);
	~TestClass();
	bool Poll();
	int PollStatus();
	void PollTracking(NullSpaceDLL::TrackingUpdate& q);
	int PlayPattern(LPSTR param, Side side);
	int PlayExperience(LPSTR param, Side side);
	int PlaySequence(LPSTR param, Location loc);
	int PlayEffect(Effect e, Location loc, float duration, float time, unsigned int priority);
	void SetTrackingEnabled(bool wantTracking);
};

