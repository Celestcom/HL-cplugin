#pragma once
#include <string>
#include "DependencyResolver.h"
#include <iostream>
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include "Wire\Wire.h"
class TestClass
{
private:
	DependencyResolver _resolver;
	Wire _wire;
public:
	TestClass(LPSTR param);
	~TestClass();
	int PlayPattern(LPSTR param, Side side);
	int PlayExperience(LPSTR param, Side side);
	int PlaySequence(LPSTR param, Location loc);
};

