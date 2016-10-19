#include "stdafx.h"
#include "TestClass.h"
#include "HapticFileInfo.h"
#include "Enums.h"

void TestClass::print()
{
}

TestClass::TestClass(LPSTR param) : _resolver(std::string(param))
{
	_context = std::make_unique<zmq::context_t>(zmq::context_t(1));
	_socket = std::make_unique<zmq::socket_t>(zmq::socket_t(*_context, ZMQ_PAIR));
	_socket->connect("tcp://127.0.0.1:5555");
	zmq::message_t request(6);
	memcpy((void *)request.data(), "Hello", 5);
	_socket->send(request);

}


TestClass::~TestClass()
{
}

int TestClass::PlayPattern(LPSTR param, Side side)
{
	return 0;
}

int TestClass::PlayExperience(LPSTR param, Side side)
{
	return 0;
}

int TestClass::PlaySequence(LPSTR param, Location loc)
{
	_resolver.Load(SequenceFileInfo(std::string(param)));
	auto res = _resolver.ResolveSequence(std::string(param), loc);
	int l = int(loc);
	zmq::message_t request(4);
	memcpy((void *)request.data(), &l, 4);
	_socket->send(&request, sizeof(l), 0);
	return 0;
}
