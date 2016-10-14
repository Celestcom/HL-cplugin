#include "stdafx.h"
#include "TestClass.h"
#include "HapticFileInfo.h"

TestClass::TestClass(LPSTR param): _resolver(std::string(param))
{
}


TestClass::~TestClass()
{
}

int TestClass::Play(LPSTR param)
{
	_resolver.Load(SequenceFileInfo(std::string(param)));
	auto res = _resolver.ResolveSequence(std::string(param), Location::Chest_Left);
	return int(res.at(0).Effect);
}
