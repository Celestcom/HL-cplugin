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


TestClass::TestClass(LPSTR param) : _resolver(std::string(param)), _wire("tcp://127.0.0.1:5555")
{



}


TestClass::~TestClass()
{
}

int TestClass::PlayPattern(LPSTR param, Side side)
{
	auto name = std::string(param);
	_resolver.Load(PatternFileInfo(name));
	auto res = _resolver.ResolvePattern(name, side);
	_wire.Send(_wire.Encode(res), name);
	
	return 0;
}

int TestClass::PlayExperience(LPSTR param, Side side)
{
	auto name = std::string(param);
	_resolver.Load(ExperienceFileInfo(name));
	auto res = _resolver.ResolvePattern(name, side);
	_wire.Send(_wire.Encode(res), name);
	return 0;
}


int TestClass::PlaySequence(LPSTR param, Location loc)
{
	auto name = std::string(param);
	_resolver.Load(SequenceFileInfo(name));
	auto res = _resolver.ResolveSequence(name, loc);
	_wire.Send(_wire.Encode(res), name);

	return 0;
}
