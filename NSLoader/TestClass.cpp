#include "stdafx.h"
#include "TestClass.h"
#include "HapticFileInfo.h"
#include "Enums.h"
#include "flatbuffers.h"
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
	flatbuffers::FlatBufferBuilder builder;
	auto effect = NullSpace::HapticFiles::CreateHapticEffect(builder, 12, 2, 125.1222, 1, 555.0);
	FinishHapticEffectBuffer(builder, effect);

	uint8_t *buf = builder.GetBufferPointer();
	int size = builder.GetSize();
	zmq::message_t msg(size);
	memcpy((void*)msg.data(), buf, size);
	_socket->send(msg);

	return 0;
}
