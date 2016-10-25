#pragma once
#include <string>
#include "DependencyResolver.h"
#include <iostream>
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include "Wire\Wire.h"
#include "Wire\EncodingOperations.h"
#pragma pack(1)
struct Quaternion {
	float w;
	float x; 
	float y;
	float z;
};

class TestClass
{
private:
	DependencyResolver _resolver;
	Wire _wire;
	EncodingOperations _encoder;
	int _suitStatus;
public:
	TestClass(LPSTR param);
	~TestClass();
	int PollStatus();
	void PollTracking(Quaternion& q);
	int PlayPattern(LPSTR param, Side side);
	int PlayExperience(LPSTR param, Side side);
	int PlaySequence(LPSTR param, Location loc);
	int PlayEffect(Effect e, Location loc, float duration, float time, unsigned int priority);
};

