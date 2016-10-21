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
	_resolver.Load(PatternFileInfo(std::string(param)));
	auto res = _resolver.ResolvePattern(std::string(param), side);
	_wire.Send(_wire.Encode(res));
	
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
	_wire.Send(_wire.Encode(res));

	return 0;
}
