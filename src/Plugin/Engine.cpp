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
#include "Locator.h"
#include "BodyView.h"
#include <chrono>
void Engine::executeTimestep(std::chrono::milliseconds dt)
{
	
	constexpr auto fraction_of_second = (1.0f / 1000.f);
	auto real_dt = dt.count() * fraction_of_second;
	
	m_player.Update(real_dt);

}

int Engine::GetHandleInfo(uint32_t m_handle, NSVR_EffectInfo* infoPtr) 
{
	if (auto info = m_player.GetHandleInfo(HapticHandle(m_handle))) {
		infoPtr->Duration = info->Duration();
		infoPtr->Elapsed = info->CurrentTime();
		infoPtr->PlaybackState = static_cast<NSVR_EffectInfo_State>(info->State());
		return NSVR_Success_Unqualified;
	}
	else {
		return NSVR_Error_NoSuchHandle;
	}
}

int Engine::GetSystems(NSVR_DeviceInfo * array, uint32_t inLength, uint32_t * outArrayLength)
{
	//todo: should be doing a timestamp check to see if connected to service.
	auto systems = m_messenger.ReadSystems();

	std::size_t upperBound = std::min<std::size_t>(inLength, systems.size());
	*outArrayLength = upperBound;
	for (std::size_t i = 0; i < upperBound; i++) {
		memcpy_s(array[i].ProductName, 128, systems[i].SystemName, 128);

	}
	return NSVR_Success_Unqualified;
}

int Engine::GetNumSystems(uint32_t * outAmount)
{
	*outAmount = m_messenger.ReadSystems().size();
	return 1;
}

Snapshot<NSVR_DeviceInfo>* Engine::TakeDeviceSnapshot()
{
	auto systems = m_messenger.ReadSystems();
	auto snapshot = std::make_unique<Snapshot<NSVR_DeviceInfo>>();
	for (const auto& system : systems) {
		NSVR_DeviceInfo info = { 0 };
		memcpy_s(info.ProductName, 128, system.SystemName, 128);
		snapshot->items.push_back(std::move(info));
	}
	snapshot->currentItem = snapshot->items.begin();

	m_snapshots.push_back(std::move(snapshot));
	return m_snapshots.back().get();
}
//
//bool Engine::IsFinishedIterating(Snapshot* snapshot) const
//{
//	auto it =std::find_if(m_snapshots.begin(), m_snapshots.end(), [snapshot](const auto& snapshotPtr) { return snapshotPtr.get() == snapshot; });
//	if (it != m_snapshots.end()) {
//		return (*it)->currentItem == (*it)->systems.end();
//	}
//	return true;
//}

void Engine::DestroyIterator(Snapshot<NSVR_DeviceInfo>* device)
{
	m_snapshots.erase(
		std::remove_if(m_snapshots.begin(), m_snapshots.end(), [device](const auto& snapshotPtr) {return snapshotPtr.get() == device; }),
		m_snapshots.end()
	);
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

	using namespace NullSpaceIPC;
	HighLevelEvent event;
	auto realtime = event.mutable_realtime_haptic();
	for (unsigned int i = 0; i < length; i++) {
		
		auto region = translator.ToRegionString(AreaFlag(areas[i]));
		auto magnitude = realtime->add_magnitudes();
		magnitude->set_region(std::move(region));
		magnitude->set_strength(static_cast<float>(strengths[i]) / 255.0f);	
	}
	m_messenger.WriteEvent(event);

	return NSVR_Success_Unqualified;
}

Engine::Engine() :

	m_isHapticsSystemPlaying(true),
	m_ioService(),
	m_messenger(m_ioService.GetIOService()),
	m_registry(),
	m_player(m_registry, m_messenger),
	m_currentHandleId(0),
	m_hapticsExecutionInterval(boost::posix_time::milliseconds(5)),
	m_hapticsTimestep(m_ioService.GetIOService(), m_hapticsExecutionInterval),
	m_lastHapticsTimestep(),
	m_cachedTrackingUpdate({})
{

	

	setupUserFacingLogSink();
	setupFileLogSink(); //not implementing this yet

	BOOST_LOG_TRIVIAL(info) << "[PluginMain] Plugin initialized";

	boost::log::core::get()->set_logging_enabled(false);

	//m_hardlightSuit = std::unique_ptr<IHapticDevice>(new HardlightDevice());
	//m_hardlightSuit->RegisterDrivers(m_registry);

	
	m_hapticsTimestep.SetEvent([this]() {
		try {
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_lastHapticsTimestep);

			executeTimestep(elapsed);
			m_lastHapticsTimestep = std::chrono::high_resolution_clock::now();
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



int Engine::UpdateView(BodyView* view)
{
	view->pairs = m_messenger.ReadBodyView();
	return NSVR_Success_Unqualified;
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

