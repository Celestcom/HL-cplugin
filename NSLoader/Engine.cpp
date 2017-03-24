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
#include "Wire\FlatbuffDecoder.h"
#include <boost\bind.hpp>
#include "EventList.h"
void Engine::executeTimestep()
{
	
	constexpr auto fraction_of_second = (1.0f / 1000.f);
	auto dt = m_hapticsExecutionInterval.total_milliseconds() * fraction_of_second;
	auto effectCommands = m_player.Update(dt);
	for (const auto& command : effectCommands) {
		m_messenger.WriteHaptics(command);
	}
	
}

Engine::Engine() :

	_decoder(std::make_unique<FlatbuffDecoder>()),
	_isEnginePlaying(true),
	m_ioService(),
	m_messenger(m_ioService.GetIOService()),
	m_player(),
	_currentHandleId(0),
	m_hapticsExecutionInterval(boost::posix_time::milliseconds(5)),
	m_hapticsExecutionTimer(m_ioService.GetIOService()),
	m_hapticsTimestep(m_ioService.GetIOService(), m_hapticsExecutionInterval),
	m_cachedTracking({})
{

	m_hapticsTimestep.SetEvent([this]() {executeTimestep(); });
	m_hapticsTimestep.Start();
	//scheduleTimestep();
}



Engine::~Engine()
{
	m_hapticsTimestep.Stop();
	m_ioService.Shutdown();
}

int Engine::PollStatus()
{
	Poll();
	if (_suitStatus == NullSpace::Communication::SuitStatus_Connected) {
		return 2; 
	}
	else {
		return 0;
	}
}

uint32_t Engine::GenHandle()
{
	//todo: bounds check
	_currentHandleId += 1;
	return _currentHandleId;
}


bool Engine::Poll() {
	if (auto optionalResponse = m_messenger.ReadSuits()) {
		auto suits = optionalResponse.get();
		if ((std::time(nullptr) - suits.timestamp) > 1) {
			_suitStatus = NullSpace::Communication::SuitStatus::SuitStatus_Disconnected;
			return true;
		}
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
	return true;
}




bool Engine::EngineCommand(NSVR_EngineCommand command)
{
	switch (command) {
	case NSVR_EngineCommand::RESUME_ALL:
		m_player.PlayAll();
		break;
	case NSVR_EngineCommand::PAUSE_ALL:
		m_player.PauseAll();
		break;
	case NSVR_EngineCommand::DESTROY_ALL:
		m_player.ClearAll();
		break;
	case NSVR_EngineCommand::ENABLE_TRACKING:
		{
		NullSpaceIPC::DriverCommand command;
		command.set_command(NullSpaceIPC::DriverCommand_Command_ENABLE_TRACKING);
		m_messenger.WriteCommand(command);
		}
		break;
	case NSVR_EngineCommand::DISABLE_TRACKING:
		{
		NullSpaceIPC::DriverCommand command;
		command.set_command(NullSpaceIPC::DriverCommand_Command_DISABLE_TRACKING);
		m_messenger.WriteCommand(command);
		}
		break;
	default:
		break;
	}
	
	return true;
}






void Engine::HandleCommand(unsigned int handle, NSVR_HandleCommand c)
{
	HapticHandle h = HapticHandle(handle);
	switch (c) {
	case NSVR_HandleCommand::PAUSE:
		m_player.Pause(h);
		break;
	case NSVR_HandleCommand::PLAY:
		m_player.Play(h);
		break;
	case NSVR_HandleCommand::RELEASE:
		m_player.Release(h);
		break;
	case NSVR_HandleCommand::RESET:
		m_player.Stop(h);
		break;
	default:
		break;
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

int Engine::CreateEffect(uint32_t handle, void* data, unsigned int size)
{
	flatbuffers::Verifier verifier(reinterpret_cast<uint8_t*>(data), size);
	if (NullSpace::Events::VerifySuitEventListBuffer(verifier)) {
		auto encodedList = NullSpace::Events::GetSuitEventList(data);
		auto decodedList = FlatbuffDecoder::Decode(encodedList);
		m_player.Create(handle, decodedList);
		return 1;
	}

	return 0;
}

int Engine::CreateEffect(EventList * list, uint32_t handle)
{
	if (list == nullptr) {
		return -1;
	}

	m_player.Create(handle, list->Events());
	return 1;


}


void copyQuaternion(NSVR_Quaternion& lhs, const NullSpace::SharedMemory::Quaternion& rhs) {
	lhs.w = rhs.w;
	lhs.x = rhs.x;
	lhs.y = rhs.y;
	lhs.z = rhs.z;
}

void copyTracking(NSVR_InteropTrackingUpdate& lhs, const NullSpace::SharedMemory::TrackingUpdate& rhs) {
	copyQuaternion(lhs.chest, rhs.chest);
	copyQuaternion(lhs.left_upper_arm, rhs.left_upper_arm);
	copyQuaternion(lhs.right_upper_arm, rhs.right_upper_arm);
}

void Engine::PollTracking(NSVR_InteropTrackingUpdate & q)
{
	q = m_cachedTracking;
	if (auto trackingUpdate = m_messenger.ReadTracking()) {
		copyTracking(q, *trackingUpdate);
		m_cachedTracking = q;
	}
}

