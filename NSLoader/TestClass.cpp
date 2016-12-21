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




TestClass::TestClass() : 
	
	_wire("tcp://127.0.0.1:9452", "tcp://127.0.0.1:9453"),
	_decoder(std::make_unique<FlatbuffDecoder>()), 
	_isEnginePlaying(true)
{



}

bool TestClass::InitializeFromFilesystem(LPSTR path) {
	//ideally we see if the filesystem exists first, and return a bool representing failure / throw an exception and 
	//catch it, translate across the interop boundary
	_resolver = std::make_unique<NodeDependencyResolver>(std::string(path));
	return true;
}


TestClass::~TestClass()
{
	EngineCommand(NullSpace::HapticFiles::EngineCommand_CLEAR_ALL);
	//TODO: figure out why we can't leave destruction totally up to zmq context destroy
}

int TestClass::PollStatus()
{
	Poll();
	return (int)_suitStatus;
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
		auto res = _resolver->Resolve(PatternArgs(name, Side::NotSpecified));
		_wire.AquireEncodingLock();
		_wire.Send(_wire.Encoder->Encode(res), res.Effect, handle);
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
		auto res = _resolver->Resolve(ExperienceArgs(name, Side::NotSpecified));
		_wire.AquireEncodingLock();
		_wire.Send(_wire.Encoder->Encode(res), res.Effect, handle);
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

bool TestClass::Load(LPSTR param, int filetype)
{
	switch (filetype) {
	case 0:
		return this->LoadSequence(param);
	case 1:
		return this->LoadPattern(param);
	case 2:
		return this->LoadExperience(param);
	}
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
		
		auto res = _resolver->Resolve(SequenceArgs(name, AreaFlag(loc), 1.0));
		_wire.AquireEncodingLock();
		
		_wire.Send(_wire.Encoder->Encode(res), res.Effect, handle);
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

void TestClass::CreateHaptic(unsigned int handle, void * data, unsigned int size)
{
	flatbuffers::Verifier verifier(reinterpret_cast<uint8_t*>(data), size);
	if (NullSpace::HapticFiles::VerifyHapticPacketBuffer(verifier)) {
		_wire.sendToEngine(reinterpret_cast<uint8_t*>(data), size);
	}
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
