#include "stdafx.h"
#include "Engine.h"
#include "Enums.h"

#include <boost\bind.hpp>
#include "EventList.h"
#include "NSLoader_Internal.h"
#include <chrono>

#include <boost/log/core.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/trivial.hpp>
#include "MyTestLog.h"

void Engine::executeTimestep()
{
	
	constexpr auto fraction_of_second = (1.0f / 1000.f);
	auto dt = m_hapticsExecutionInterval.total_milliseconds() * fraction_of_second;
	auto effectCommands = m_player.Update(dt);
	for (const auto& command : effectCommands) {
		m_messenger.WriteHaptics(command);
	}
	
}

int Engine::DumpDeviceDiagnostics()
{
	NullSpaceIPC::DriverCommand command;
	command.set_command(NullSpaceIPC::DriverCommand_Command_DUMP_DEVICE_DIAGNOSTICS);
	m_messenger.WriteCommand(command);
	return NSVR_Success_Unqualified;
}

Engine::Engine() :

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

	m_hapticsTimestep.SetEvent([this]() {
		try {
			executeTimestep();
		}
		catch (const std::exception& e) {
			BOOST_LOG_TRIVIAL(error) << "[PluginMain] Fatal error executing timestep: " << e.what();

		}
	});
	m_hapticsTimestep.Start();

	using namespace boost::log;
	m_log = boost::make_shared<MyTestLog>();
	m_log->Provide(&m_messenger, m_ioService.GetIOService());
	typedef sinks::synchronous_sink<MyTestLog> sink_t;

	boost::shared_ptr<sink_t> sink(new sink_t(m_log));
	
	//sink->locked_backend()->Provide(&m_messenger, m_ioService.GetIOService());

	core::get()->add_sink(sink);

	BOOST_LOG_TRIVIAL(info) << "[PluginMain] Plugin initialized";

}



Engine::~Engine()
{
	try {
		m_player.ClearAll();
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
		m_hapticsTimestep.Stop();
		m_ioService.Shutdown();
	}
	catch (const std::exception&) {

	}
}

int Engine::PollStatus(NSVR_ServiceInfo* info)
{
	if (m_messenger.ConnectedToService(info)) {
	
			return NSVR_Success_Unqualified;
		
	}

	return NSVR_Error_ServiceDisconnected;

	
}

uint32_t Engine::GenHandle()
{
	//todo: bounds check
	_currentHandleId += 1;
	return _currentHandleId;
}

int Engine::PollDevice(NSVR_DeviceInfo * device)
{
	//when polling for devices, we want to either fill in the device info struct or return an error if no devices present.
	
	if (auto optionalResponse = m_messenger.ReadSuits()) {
		auto suits = optionalResponse.get();
		if ((std::time(nullptr) - suits.timestamp) > 1) {
			return NSVR_Error_ServiceDisconnected;
		}
		for (int i = 0; i < 4; i++) {
			if (suits.SuitsFound[i]) {
				auto sStatus = suits.Suits[i].Status;
				if (sStatus == NullSpace::SharedMemory::Connected) {
					//here, we'd make sure to check if device == nullptr if we were filling the struct.
					//The user doesn't care about the results, so we just return success.


					return NSVR_Success_Unqualified;
				}
			}
		}
		return NSVR_Error_NoDevicePresent;
	} 

	return NSVR_Error_ServiceDisconnected;
}





bool Engine::EngineCommand(NSVR_EngineCommand command)
{
	switch (command) {
	case NSVR_EngineCommand::NSVR_EngineCommand_ResumeAll:
		m_player.PlayAll();
		break;
	case NSVR_EngineCommand::NSVR_EngineCommand_PauseAll:
		m_player.PauseAll();
		break;
	case NSVR_EngineCommand::NSVR_EngineCommand_DestroyAll:
		m_player.ClearAll();
		break;
	case NSVR_EngineCommand::NSVR_EngineCommand_EnableTracking:
		{
		NullSpaceIPC::DriverCommand command;
		command.set_command(NullSpaceIPC::DriverCommand_Command_ENABLE_TRACKING);
		m_messenger.WriteCommand(command);
		}
		break;
	case NSVR_EngineCommand::NSVR_EngineCommand_DisableTracking:
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

int Engine::GetEngineStats(NSVR_SystemStats * stats)
{
	stats->NumLiveEffects = static_cast<unsigned int>(m_player.NumLiveEffects());
	stats->NumOrphanedEffects = static_cast<unsigned int>(m_player.NumOrphanedEffects());
	return NSVR_Success_Unqualified;
}






void Engine::HandleCommand(unsigned int handle, NSVR_PlaybackCommand c)
{
	HapticHandle h = HapticHandle(handle);
	switch (c) {
	case NSVR_PlaybackCommand::NSVR_PlaybackCommand_Pause:
		m_player.Pause(h);
		break;
	case NSVR_PlaybackCommand::NSVR_PlaybackCommand_Play:
		m_player.Play(h);
		break;
	case 3: //release
		m_player.Release(h);
		break;
	case NSVR_PlaybackCommand::NSVR_PlaybackCommand_Reset:
		m_player.Stop(h);
		break;
	default:
		break;
	}
	
}



void Engine::GetError(NSVR_ErrorInfo* errorInfo)
{
	strncpy_s(errorInfo->ErrorString, 512, _currentError.c_str(), 512);
	errorInfo->ErrorString[511] = '\0';
	//todo: Keep track of LastResult as well
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

void copyTracking(NSVR_TrackingUpdate& lhs, const NullSpace::SharedMemory::TrackingUpdate& rhs) {
	copyQuaternion(lhs.chest, rhs.chest);
	copyQuaternion(lhs.left_upper_arm, rhs.left_upper_arm);
	copyQuaternion(lhs.right_upper_arm, rhs.right_upper_arm);
}

int Engine::PollTracking(NSVR_TrackingUpdate* q)
{
	
	if (auto trackingUpdate = m_messenger.ReadTracking()) {
		copyTracking(*q, *trackingUpdate);
		return NSVR_Success_Unqualified;
	}
	else {
		return NSVR_Success_NoDataAvailable;
	}
}

int Engine::PollLogs(NSVR_LogEntry * entry)
{
	if (auto logEntry = m_log->Poll()) {
		auto str = *logEntry;
		entry->Length = static_cast<uint32_t>(str.length());
		strncpy_s(entry->Message, 512, str.c_str(), 512);
		entry->Message[511] = '\0';
		return NSVR_Success_Unqualified;
	}
	else {
		return NSVR_Success_NoDataAvailable;
	}
}

int Engine::EnableAudio(NSVR_AudioOptions* optionsPtr)
{
	NullSpaceIPC::DriverCommand command;
	command.set_command(NullSpaceIPC::DriverCommand_Command_ENABLE_AUDIO);
	if (optionsPtr == nullptr) {
		(*command.mutable_params())["audio_min"] = 0x04;
		(*command.mutable_params())["audio_max"] = 0x22;
		(*command.mutable_params())["peak_time"] = 0x01;
		(*command.mutable_params())["filter"] = 0x01;
	}
	else
	{
		(*command.mutable_params())["audio_min"] = std::min(std::max<int>(0, optionsPtr->AudioMin), 255);
		(*command.mutable_params())["audio_max"] = std::min(std::max<int>(0, optionsPtr->AudioMax), 255);
		(*command.mutable_params())["peak_time"] = std::min(std::max<int>(0, optionsPtr->PeakTime), 3);
		(*command.mutable_params())["filter"] = std::min(std::max<int>(0, optionsPtr->Filter), 3);
	}
	m_messenger.WriteCommand(command);
	return NSVR_Success_Unqualified;
}

int Engine::DisableAudio()
{
	NullSpaceIPC::DriverCommand command;
	command.set_command(NullSpaceIPC::DriverCommand_Command_DISABLE_AUDIO);
	m_messenger.WriteCommand(command);
	return NSVR_Success_Unqualified;
}

int Engine::SubmitRawCommand(uint8_t * buffer, int length)
{
	NullSpaceIPC::DriverCommand command;
	command.set_command(NullSpaceIPC::DriverCommand_Command_RAW_COMMAND);
	command.set_raw_command(buffer, length);
	m_messenger.WriteCommand(command);
	return NSVR_Success_Unqualified;
}

