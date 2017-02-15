#include "stdafx.h"
#include "Engine.h"
#include "HapticFileInfo.h"
#include "Enums.h"
#include "flatbuffers\flatbuffers.h"

#include "Sequence_generated.h"
#include "Pattern_generated.h"

#include "HapticEffect_generated.h"
#include "HapticFrame_generated.h"
#include "HapticPacket_generated.h"
#include "Wire\EncodingOperations.h"
#include "Wire\FlatbuffDecoder.h"


void Engine::scheduleTimestep()
{
	m_hapticsExecutionTimer.expires_from_now(m_hapticsExecutionInterval);
	m_hapticsExecutionTimer.async_wait(boost::bind(&Engine::executeTimestep, this, boost::asio::placeholders::error));
}

void Engine::executeTimestep(const boost::system::error_code & ec)
{
	auto dt = float( m_hapticsExecutionInterval.fractional_seconds())/1000.0f;
	m_player.Update(dt);
	scheduleTimestep();
}

Engine::Engine() :
	
	_decoder(std::make_unique<FlatbuffDecoder>()), 
	_isEnginePlaying(true),
	m_ioService(),
	m_messenger(m_ioService.GetIOService()),
	m_player(),
	m_hapticsExecutionInterval(10),
	m_hapticsExecutionTimer(m_ioService.GetIOService())
{



}

bool Engine::InitializeFromFilesystem(LPSTR path) {
	//ideally we see if the filesystem exists first, and return a bool representing failure / throw an exception and 
	//catch it, translate across the interop boundary
	_resolver = std::make_unique<NodeDependencyResolver>(std::string(path));
	return true;
}



Engine::~Engine()
{
	//EngineCommand(NullSpace::HapticFiles::EngineCommand_CLEAR_ALL);
	m_ioService.Shutdown();
}

int Engine::PollStatus()
{
	Poll();
	//return (int)_suitStatus;
	return 1;
}

uint32_t Engine::GenHandle()
{
	return _currentHandleId++;
}


bool Engine::Poll() {
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



bool Engine::GetPlayingStatus()
{
	return _isEnginePlaying;
}



bool Engine::EngineCommand(short command)
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





void Engine::SetTrackingEnabled(bool wantTracking)
{
	//_wire.AquireEncodingLock();
	//_wire.Send(_wire.Encoder->Encode(wantTracking));
	//_wire.ReleaseEncodingLock();

}

void Engine::HandleCommand(unsigned int handle, short c)
{
	//_wire.AquireEncodingLock();
	//_wire.Send(_wire.Encoder->Encode(NullSpaceDLL::HandleCommand(handle, c)));
	//_wire.ReleaseEncodingLock();
}

//coming in as nodal formatted data..?
void Engine::CreateHaptic(unsigned int handle, void * data, unsigned int size)
{
	flatbuffers::Verifier verifier(reinterpret_cast<uint8_t*>(data), size);
	if (NullSpace::HapticFiles::VerifyHapticPacketBuffer(verifier)) {
		
		auto packet = NullSpace::HapticFiles::GetHapticPacket(data);
		auto type = packet->packet_type();
		auto  n = static_cast<const NullSpace::HapticFiles::Node*>(packet->packet());
		auto node = EncodingOperations::Decode(n);
		
		//Push all values down, such as strength and time offset
		NullSpace::Propogate(node);
		//Pull out all effects
		auto effects = NullSpace::Flatten(node);
		int a = 3;
	}
}

char* Engine::GetError()
{
	const char* sampleString = _currentError.c_str();
	std::size_t len = strlen(sampleString) + sizeof(char);
	char* newString = new char[len];
	newString[len - 1] = 0;
	strcpy_s(newString, len, sampleString);


	return newString;
}
