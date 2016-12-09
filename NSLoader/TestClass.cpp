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




TestClass::TestClass(LPSTR param) : 
	_resolver(std::make_unique<DependencyResolver>(std::string(param))), 
	_wire("tcp://127.0.0.1:9452", "tcp://127.0.0.1:9453"),
	_decoder(std::make_unique<FlatbuffDecoder>()), 
	_isEnginePlaying(true)
{



}

TestClass::TestClass():_wire("tcp://127.0.0.1:9452", "tcp://127.0.0.1:9453")
{
}


TestClass::~TestClass()
{

	//TODO: figure out why we can't leave destruction totally up to zmq context destroy
}

/* returns 1 for connected, 0 for disconnected */
int TestClass::PollStatus()
{
	Poll();
	return _suitStatus;
}

uint32_t TestClass::GenHandle()
{
	return _currentHandleId++;
}

bool TestClass::LoadSequence(LPSTR param)
{
	try {
		return _resolver->Load(SequenceFileInfo(std::string(param)));

	}
	catch (const InvalidPackageNameException& e) {
		_currentError = std::string(param) + ": " + e.what();
	}
	catch (const PackageNotFoundException& e) {
		_currentError = std::string(param) + ": " + e.what();
	}
	catch (const FileNotFoundException& e) {
		_currentError = std::string(param) + ": " + e.what();
	}
	catch (const MalformedHapticsFileException& m) {
		_currentError =  m.what();
	}
	catch (Json::Exception& e) {
		_currentError = std::string(param) + ": " + e.what() + " (Invalid JSON syntax)";
	}
	return false;
}

bool TestClass::LoadPattern(LPSTR param)
{
	try {
		return _resolver->Load(PatternFileInfo(std::string(param)));
		
	}
	catch (const InvalidPackageNameException& e) {
		_currentError = std::string(param) + ": " + e.what();
	}
	catch (const PackageNotFoundException& e) {
		_currentError = std::string(param) + ": " + e.what();
	}
	catch (const FileNotFoundException& e) {
		_currentError = std::string(param) + ": " + e.what();
	}
	catch (const MalformedHapticsFileException& m) {
		_currentError = m.what();
	}
	catch (Json::Exception& e) {
		_currentError = std::string(param) +  ": " + e.what() + " (Invalid JSON syntax)";
	}

	return false;

}

bool TestClass::CreatePattern(uint32_t handle, LPSTR param)
{
	auto name = std::string(param);
	if (_resolver->Load(PatternFileInfo(name))) {
		auto res = _resolver->ResolvePattern(name);
		_wire.AquireEncodingLock();
		_wire.Send(_wire.Encoder->Encode(res), res.Name(), handle);
		_wire.ReleaseEncodingLock();
		return true;
	}
	return false;
}

bool TestClass::LoadExperience(LPSTR param)
{
	try {
		return _resolver->Load(ExperienceFileInfo(std::string(param)));
	}
	
	catch (const InvalidPackageNameException& e) {
		_currentError = std::string(param) + ": " + e.what();
	}
	catch (const PackageNotFoundException& e) {
		_currentError = std::string(param) + ": " + e.what();
	}
	catch (const FileNotFoundException& e) {
		_currentError = std::string(param) + ": " + e.what();
	}
	catch (const MalformedHapticsFileException& m) {
		_currentError = m.what();
	}
	catch (Json::Exception& e) {
		_currentError = std::string(param) + ": " + e.what() + " (Invalid JSON syntax)";
	}
	return false;
}

bool TestClass::CreateExperience(uint32_t handle, LPSTR param)
{
	auto name = std::string(param);
	if (_resolver->Load(ExperienceFileInfo(name))) {
		auto res = _resolver->ResolveExperience(name);
		_wire.AquireEncodingLock();
		_wire.Send(_wire.Encoder->Encode(res), res.Name(), handle);
		_wire.ReleaseEncodingLock();
		return true;
	}
	return false;
}

bool TestClass::Poll() {
	return _wire.Receive(_suitStatus, _tracking);
}

void TestClass::PollTracking(NullSpaceDLL::InteropTrackingUpdate& t) {
	Poll();
	t = _tracking;
}

bool TestClass::GetPlayingStatus()
{
	return _isEnginePlaying;
}

bool TestClass::EngineCommand(short command)
{
	if (command == NullSpace::HapticFiles::EngineCommand_PLAY_ALL) {
		//update our cached state

		_isEnginePlaying = true;
	}
	else if (command == NullSpace::HapticFiles::EngineCommand_PAUSE_ALL) {
		//update our cached state
		_isEnginePlaying = false;
	}

	_wire.AquireEncodingLock();
	_wire.Send(_wire.Encoder->Encode(NullSpaceDLL::EngineCommand(command)));
	_wire.ReleaseEncodingLock();
	return true;
}



//TODO: wrap with exception handling incase flatbuffers fails/file fails to load/etc
bool TestClass::CreateSequence(uint32_t handle, LPSTR param, uint32_t loc)
{
	//strengths need to keep propogating everywhere. Tehn need to do it in engine
	auto name = std::string(param);
	if (_resolver->Load(SequenceFileInfo(name))) {
		//default strength for now
		auto res = _resolver->ResolveSequence(name, AreaFlag(loc), 1.0);
		_wire.AquireEncodingLock();
		
		_wire.Send(_wire.Encoder->Encode(res), res.Name(), handle);
		_wire.ReleaseEncodingLock();
		return true;
	}
	return false;
}


void TestClass::SetTrackingEnabled(bool wantTracking)
{
	_wire.AquireEncodingLock();
	_wire.Send(_wire.Encoder->Encode(wantTracking));
	_wire.ReleaseEncodingLock();

}

void TestClass::HandleCommand(unsigned int handle, short c)
{
	_wire.AquireEncodingLock();
	_wire.Send(_wire.Encoder->Encode(NullSpaceDLL::HandleCommand(handle, c)));
	_wire.ReleaseEncodingLock();
}

char* TestClass::GetError()
{
	const char* sampleString = _currentError.c_str();
	std::size_t len = strlen(sampleString) + sizeof(char);
	char* newString = new char[len];
	newString[len - 1] = 0;
	strcpy_s(newString, len, sampleString);


	return newString;
}
