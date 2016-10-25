#include "stdafx.h"
#include "TestClass.h"
#include "HapticFileInfo.h"
#include "Enums.h"
#include "flatbuffers\flatbuffers.h"

#include "Sequence_generated.h"
#include "Pattern_generated.h"

#include "HapticEffect_generated.h"
#include "HapticFrame_generated.h"
#include "HapticPacket_generated.h"


TestClass::TestClass(LPSTR param) : _resolver(std::string(param)), _wire("tcp://127.0.0.1:9452", "tcp://127.0.0.1:9453")
{



}


TestClass::~TestClass()
{
	//TODO: figure out why we can't leave destruction totally up to zmq context destroy
}

/* returns 1 for connected, 0 for disconnected */
int TestClass::PollStatus()
{
	NullSpace::Communication::SuitStatus status;
	if (_wire.ReceiveStatus(&status)) {
		_suitStatus = status == NullSpace::Communication::SuitStatus::SuitStatus_Connected ? 1 : 0;
	}
	return _suitStatus;
}

void TestClass::PollTracking(Quaternion& q)
{
	q.w = 1.0;
	q.x = 2.9;
	q.y = 3.0;
	q.z = 4.0;
}

int TestClass::PlayPattern(LPSTR param, Side side)
{
	auto name = std::string(param);
	if (_resolver.Load(PatternFileInfo(name))) {
		auto res = _resolver.ResolvePattern(name, side);
		_wire.Send(_encoder.Encode(res), name);
	}
	return 0;
}

int TestClass::PlayExperience(LPSTR param, Side side)
{
	auto name = std::string(param);
	if (_resolver.Load(ExperienceFileInfo(name))) {
		auto res = _resolver.ResolvePattern(name, side);
		_wire.Send(_encoder.Encode(res), name);
	}
	return 0;
}


int TestClass::PlaySequence(LPSTR param, Location loc)
{
	auto name = std::string(param);
	if (_resolver.Load(SequenceFileInfo(name))) {
		auto res = _resolver.ResolveSequence(name, loc);
		_wire.Send(_encoder.Encode(res), name);
	}
	return 0;
}

int TestClass::PlayEffect(Effect e, Location loc, float duration, float time, unsigned int priority)
{
	_wire.Send(_encoder.Encode(HapticEffect(e, loc, duration, time, priority)));
	return 0;
}
