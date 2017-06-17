#include "stdafx.h"
#include "Engine.h"
#include "Enums.h"
#include <boost/log/support/date_time.hpp>
#include <boost\bind.hpp>
#include "EventList.h"
#include "NSLoader_Internal.h"
#include <chrono>

#include <boost/log/core.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/trivial.hpp>
#include <boost/core/null_deleter.hpp>
#include "MyTestLog.h"
#include "IHapticDevice.h"
#include "Devices/HardlightDevice/hardlightdevice.h"
#include "Locator.h"

void Engine::executeTimestep()
{
	
	constexpr auto fraction_of_second = (1.0f / 1000.f);
	auto dt = m_hapticsExecutionInterval.total_milliseconds() * fraction_of_second;
	
	m_player.Update(dt);
	auto commands = m_hardlightSuit->GenerateHardwareCommands(dt);
	
	for (const auto& command : commands) {
		m_messenger.WriteHaptics(command);
	}
	
}

int Engine::GetHandleInfo(uint32_t m_handle, NSVR_HandleInfo* infoPtr) 
{
	if (auto info = m_player.GetHandleInfo(HapticHandle(m_handle))) {
		infoPtr->Duration = info->Duration();
		infoPtr->Elapsed = info->CurrentTime();
		return NSVR_Success_Unqualified;
	}
	else {
		return NSVR_Error_NoSuchHandle;
	}
}

int Engine::DumpDeviceDiagnostics()
{
	NullSpaceIPC::DriverCommand command;
	command.set_command(NullSpaceIPC::DriverCommand_Command_DUMP_DEVICE_DIAGNOSTICS);
	m_messenger.WriteCommand(command);
	return NSVR_Success_Unqualified;
}

int Engine::SetStrengths(uint16_t* strengths, uint32_t* areas, unsigned int length)
{
	auto& translator = Locator::getTranslator();
	for (unsigned int i = 0; i < length; i++) {
		
		auto drivers = m_registry.GetRtpDrivers(translator.ToRegionString(AreaFlag(areas[i])));

		if (drivers) {
			std::for_each(drivers->begin(), drivers->end(), [&](auto& driver) {
				driver->realtime(RealtimeArgs(strengths[i]));
			});
		}
	}

	return NSVR_Success_Unqualified;
}

Engine::Engine() :

	m_isHapticsSystemPlaying(true),
	m_ioService(),
	m_messenger(m_ioService.GetIOService()),
	m_registry(),
	m_player(m_registry),
	m_currentHandleId(0),
	m_hapticsExecutionInterval(boost::posix_time::milliseconds(5)),
	m_hapticsTimestep(m_ioService.GetIOService(), m_hapticsExecutionInterval),
	m_cachedTrackingUpdate({})
{

	

	setupUserFacingLogSink();
	setupFileLogSink(); //not implementing this yet

	BOOST_LOG_TRIVIAL(info) << "[PluginMain] Plugin initialized";

	boost::log::core::get()->set_logging_enabled(false);

	m_hardlightSuit = std::unique_ptr<IHapticDevice>(new HardlightDevice());
	m_hardlightSuit->RegisterDrivers(m_registry);


	m_hapticsTimestep.SetEvent([this]() {
		try {
			executeTimestep();
		}
		catch (const std::exception& e) {
			BOOST_LOG_TRIVIAL(error) << "[PluginMain] Fatal error executing timestep: " << e.what();

		}
	});
	m_hapticsTimestep.Start();
}


void Engine::setupFileLogSink()
{
	using namespace boost::log;
	auto backend = boost::make_shared<sinks::text_file_backend>(
		  keywords::file_name = "nsvr_plugin_%5N.log"
		, keywords::rotation_size = 5 * 1024 * 1024
		, keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0)

	);


	typedef sinks::synchronous_sink<sinks::text_file_backend> sink_t;
	boost::shared_ptr<sink_t> sink(new sink_t(backend));

	boost::log::core::get()->add_sink(sink);

}



void Engine::setupUserFacingLogSink()
{
	
	using namespace boost::log;
	m_log = boost::make_shared<MyTestLog>();
	m_log->Provide(&m_messenger, m_ioService.GetIOService());
	
	typedef sinks::synchronous_sink<MyTestLog> sink_t;
	boost::shared_ptr<sink_t> sink(new sink_t(m_log));
	core::get()->add_sink(sink);
	
}

Engine::~Engine()
{

	boost::log::core::get()->remove_all_sinks();

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

uint32_t Engine::GenerateHandle()
{
	//todo: bounds check
	m_currentHandleId += 1;
	return m_currentHandleId;
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





bool Engine::DoEngineCommand(::EngineCommand command)
{
	switch (command) {
	case EngineCommand::ResumeAll:
		m_player.PlayAll();
		break;
	case EngineCommand::PauseAll:
		m_player.PauseAll();
		break;
	case EngineCommand::DestroyAll:
		m_player.ClearAll();
		break;
	case EngineCommand::EnableTracking:
		{
		NullSpaceIPC::DriverCommand command;
		command.set_command(NullSpaceIPC::DriverCommand_Command_ENABLE_TRACKING);
		m_messenger.WriteCommand(command);
		}
		break;
	case EngineCommand::DisableTracking:
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
	stats->NumLiveEffects = static_cast<unsigned int>(m_player.GetNumLiveEffects());
	stats->NumOrphanedEffects = static_cast<unsigned int>(m_player.GetNumReleasedEffects());
	return NSVR_Success_Unqualified;
}



void Engine::ReleaseHandle(unsigned int handle)
{
	HapticHandle h = HapticHandle(handle);
	m_player.Release(h);
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




std::vector<std::unique_ptr<PlayableEvent>> 
extractPlayables(const std::vector<ParameterizedEvent>& events) {
	
	using PlayablePtr = std::unique_ptr<PlayableEvent>;
	std::vector<PlayablePtr> playables;
	playables.reserve(events.size());
	for (const auto& event : events) {
		if (auto newPlayable = PlayableEvent::make(event.type())) {
			if (newPlayable->parse(event)) {
				playables.push_back(std::move(newPlayable));
			}
		}
	}

	return playables;
}

//Only modifies handle if the effect is created successfully
int Engine::CreateEffect(EventList * list, HapticHandle* handle)
{
	if (list == nullptr) {
		return NSVR_Error_NullArgument;
	}
	//enforces precondition on PlayableEffect to not have an empty effects list
	if (list->empty()) {
		return -1;
	}
	else {
		HapticHandle h = m_player.Create(extractPlayables(list->events()));
		*handle = h;
		return NSVR_Success_Unqualified;
	}

	return NSVR_Error_Unknown;


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

int Engine::Sample(uint16_t * strengths, uint32_t * areas, uint32_t* families, int length, unsigned int * resultCount)
{
	auto samples = m_hardlightSuit->QueryDrivers();

	std::size_t max_num_effects = std::min<std::size_t>(length, samples.size());
	for (std::size_t i = 0; i < max_num_effects; i++) {
		const auto& effect = samples[i];
	
		strengths[i] = effect.strength;
		families[i] = effect.family;
		areas[i] = static_cast<uint32_t>(effect.area);

	}

	*resultCount = static_cast<unsigned int>(samples.size());

	return NSVR_Success_Unqualified;
}

