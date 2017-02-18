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
	if (!ec) {
		//dt = total_ms * (1/1000) seconds 
		constexpr auto fraction_of_second = (1.0f / 1000.f);
		auto dt = m_hapticsExecutionInterval.total_milliseconds() * fraction_of_second;
		auto effectCommands = m_player.Update(dt);
		for (const auto& command : effectCommands) {
			m_messenger.WriteHaptics(command);
		}
		scheduleTimestep();
	}
}

Engine::Engine() :
	
	_decoder(std::make_unique<FlatbuffDecoder>()), 
	_isEnginePlaying(true),
	m_ioService(),
	m_messenger(m_ioService.GetIOService()),
	m_player(),
	m_hapticsExecutionInterval(boost::posix_time::milliseconds(10)),
	m_hapticsExecutionTimer(m_ioService.GetIOService())
{


	scheduleTimestep();
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
	HapticHandle h = HapticHandle(handle);
	switch (NullSpace::HapticFiles::Command(c)) {
	case NullSpace::HapticFiles::Command_PAUSE:
		m_player.Pause(h);
		break;
	case NullSpace::HapticFiles::Command_PLAY:
		m_player.Play(h);
		break;
	case NullSpace::HapticFiles::Command_RELEASE:
		m_player.Release(h);
		break;
	case NullSpace::HapticFiles::Command_RESET:
		m_player.Stop(h);
		break;
	default:
		break;
	}
	
}

//coming in as nodal formatted data..?
void Engine::CreateHaptic(unsigned int handle, void * data, unsigned int size)
{
	flatbuffers::Verifier verifier(reinterpret_cast<uint8_t*>(data), size);
	if (NullSpace::HapticFiles::VerifyHapticPacketBuffer(verifier)) {
		
		auto packet = NullSpace::HapticFiles::GetHapticPacket(data);
		if (packet->packet_type() != NullSpace::HapticFiles::FileType::FileType_Node) {
			return;
		}

		auto node = EncodingOperations::Decode(static_cast<const NullSpace::HapticFiles::Node*>(packet->packet()));
		
		//Push all values down, such as strength and time offset
		NullSpace::Propogate(node);
		//Pull out all effects
		auto effects = NullSpace::Flatten(node);

		//Encode the nodes as TinyEffects for use in engine
		auto tinyEffects = NullSpace::EncodeTinyEffects(effects);


		m_player.Create(handle, tinyEffects);
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
