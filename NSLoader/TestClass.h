#pragma once
#include <string>
#include "DependencyResolver.h"
#include <iostream>
#include "zmq.hpp"

class TestClass
{
private:
	DependencyResolver _resolver;
	std::unique_ptr<zmq::socket_t> _socket;
	std::unique_ptr<zmq::context_t> _context;
	void print();
public:
	TestClass(LPSTR param);
	~TestClass();
	int PlayPattern(LPSTR param, Side side);
	int PlayExperience(LPSTR param, Side side);
	int PlaySequence(LPSTR param, Location loc);
};

