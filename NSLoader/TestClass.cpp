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

#include "Wire\FlatbuffDecoder.h"


TestClass::TestClass() : 
	
	_decoder(std::make_unique<FlatbuffDecoder>()), 
	_isEnginePlaying(true),
	m_ioService(),
	m_messenger(m_ioService.GetIOService())
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
	//EngineCommand(NullSpace::HapticFiles::EngineCommand_CLEAR_ALL);
	m_ioService.Shutdown();
}

int TestClass::PollStatus()
{
	Poll();
	//return (int)_suitStatus;
	return 1;
}

uint32_t TestClass::GenHandle()
{
	return _currentHandleId++;
}


bool TestClass::Poll() {
	if (auto optionalResponse = m_messenger.ReadSuits()) {
		auto suits = optionalResponse.get();
		for (int i = 0; i < 4; i++) {
			if (suits.SuitsFound[i]) {
				auto status  = suits.Suits[i].Status;
				if (status == NullSpace::SharedMemory::Connected) {
					_suitStatus = NullSpace::Communication::SuitStatus::SuitStatus_Connected;
				}
				else {
					_suitStatus = NullSpace::Communication::SuitStatus::SuitStatus_Disconnected;
				}
			}
		}
	}
	//return _wire.Receive(_suitStatus, _tracking);
	return true;
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

	//_wire.AquireEncodingLock();
	//_wire.Send(_wire.Encoder->Encode(NullSpaceDLL::EngineCommand(command)));
	//_wire.ReleaseEncodingLock();
	return true;
}





void TestClass::SetTrackingEnabled(bool wantTracking)
{
	//_wire.AquireEncodingLock();
	//_wire.Send(_wire.Encoder->Encode(wantTracking));
	//_wire.ReleaseEncodingLock();

}

void TestClass::HandleCommand(unsigned int handle, short c)
{
	//_wire.AquireEncodingLock();
	//_wire.Send(_wire.Encoder->Encode(NullSpaceDLL::HandleCommand(handle, c)));
	//_wire.ReleaseEncodingLock();
}

void TestClass::CreateHaptic(unsigned int handle, void * data, unsigned int size)
{
	flatbuffers::Verifier verifier(reinterpret_cast<uint8_t*>(data), size);
	if (NullSpace::HapticFiles::VerifyHapticPacketBuffer(verifier)) {
	//	_wire.sendToEngine(reinterpret_cast<uint8_t*>(data), size);
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
