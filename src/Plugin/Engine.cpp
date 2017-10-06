#include "stdafx.h"
#include "Engine.h"
#include "Enums.h"
#include <boost/log/support/date_time.hpp>
#include <boost\bind.hpp>
#include "EventList.h"
#include "HLVR_Experimental.h"
#include <chrono>

#include <boost/log/core.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/trivial.hpp>
#include <boost/core/null_deleter.hpp>
#include "MyTestLog.h"
#include "Locator.h"
#include "BodyView.h"
#include <chrono>
void Engine::executeTimestep(std::chrono::milliseconds dt)
{
	
	constexpr auto fraction_of_second = (1.0f / 1000.f);
	auto real_dt = dt.count() * fraction_of_second;
	
	m_player.Update(real_dt);

}

int Engine::GetHandleInfo(uint32_t m_handle, HLVR_EffectInfo* infoPtr) 
{
	if (auto info = m_player.GetHandleInfo(HapticHandle(m_handle))) {
		infoPtr->Duration = info->Duration();
		infoPtr->Elapsed = info->CurrentTime();
		infoPtr->PlaybackState = static_cast<HLVR_EffectInfo_State>(info->State());
		return HLVR_Ok;
	}
	else {
		return HLVR_Error_NoSuchHandle;
	}
}



int Engine::GetNumDevices(uint32_t * outAmount)
{
	*outAmount = m_messenger.ReadDevices().size();
	return 1;
}

HiddenIterator<HLVR_DeviceInfo>* Engine::TakeDeviceSnapshot()
{
	auto systems = m_messenger.ReadDevices();

	std::vector<HLVR_DeviceInfo> devices;
	for (const auto& system : systems) {
		HLVR_DeviceInfo deviceInfo = { 0 };
		memcpy_s(deviceInfo.Name, 128, system.DeviceName, 128);
		deviceInfo.Status = static_cast<HLVR_DeviceStatus>(system.Status);
		deviceInfo.Id = system.Id;
		deviceInfo.Concept = static_cast<HLVR_DeviceConcept>(system.Concept);
		devices.push_back(std::move(deviceInfo));
	}
	
	auto snapshot = std::make_unique<HiddenIterator<HLVR_DeviceInfo>>(devices);
	m_deviceSnapshots.push_back(std::move(snapshot));
	return m_deviceSnapshots.back().get();
}

HiddenIterator<HLVR_NodeInfo>* Engine::TakeNodeSnapshot(uint32_t device_id)
{
	auto nodes_raw = m_messenger.ReadNodes();
	std::vector<HLVR_NodeInfo> nodes;
	for (const auto& node : nodes_raw) {
		//special case: they want ALL nodes, regardless of device, then specify device_id 0
		if ((node.DeviceId) == device_id || device_id == 0) {
			HLVR_NodeInfo nodeInfo = { 0 };
			memcpy_s(nodeInfo.Name, 128, node.NodeName, 128);
			nodeInfo.Id = node.Id;
			nodeInfo.Concept = static_cast<HLVR_NodeConcept>(node.Type);
			nodes.push_back(std::move(nodeInfo));
		}
	}

	auto snapshot = std::make_unique<HiddenIterator<HLVR_NodeInfo>>(nodes);
	m_nodeSnapshots.push_back(std::move(snapshot));
	return m_nodeSnapshots.back().get();
}

void Engine::DestroyIterator(HiddenIterator<HLVR_DeviceInfo>* device)
{
	m_deviceSnapshots.erase(
		std::remove_if(m_deviceSnapshots.begin(), m_deviceSnapshots.end(), [device](const auto& snapshotPtr) {return snapshotPtr.get() == device; }),
		m_deviceSnapshots.end()
	);
}


void Engine::DestroyIterator(HiddenIterator<HLVR_NodeInfo>* nodes)
{
	m_nodeSnapshots.erase(
		std::remove_if(m_nodeSnapshots.begin(), m_nodeSnapshots.end(), [nodes](const auto& snapshotPtr) {return snapshotPtr.get() == nodes; }),
		m_nodeSnapshots.end()
	);
}

int Engine::DumpDeviceDiagnostics()
{
	NullSpaceIPC::DriverCommand command;
	command.set_command(NullSpaceIPC::DriverCommand_Command_DUMP_DEVICE_DIAGNOSTICS);
	m_messenger.WriteCommand(command);
	return HLVR_Ok;
}

int Engine::SetStrengths(uint32_t* regions, double* amplitudes, uint32_t length)
{
	throw std::runtime_error("reimplement");
	/*auto& translator = Locator::getTranslator();

	using namespace NullSpaceIPC;
	HighLevelEvent event;
	auto realtime = event.mutable_realtime_haptic();
	for (unsigned int i = 0; i < length; i++) {
		auto magnitude = realtime->add_magnitudes();
		magnitude->set_region(regions[i]);
		magnitude->set_strength(amplitudes[i]);	
	}
	m_messenger.WriteEvent(event);
*/
	return HLVR_Ok;
}

Engine::Engine() :

	m_isHapticsSystemPlaying(true),
	m_ioService(),
	m_messenger(m_ioService.GetIOService()),
	m_player( m_messenger),
	m_currentHandleId(0),
	m_hapticsExecutionInterval(boost::posix_time::milliseconds(5)),
	m_hapticsTimestep(m_ioService.GetIOService(), m_hapticsExecutionInterval),
	m_lastHapticsTimestep(),
	m_cachedTrackingUpdate({}),
	m_deviceSnapshots(),
	m_nodeSnapshots()
{

	

	setupUserFacingLogSink();
	setupFileLogSink(); //not implementing this yet

	BOOST_LOG_TRIVIAL(info) << "[PluginMain] Plugin initialized";

	boost::log::core::get()->set_logging_enabled(false);

	
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
	return HLVR_Ok;
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

int Engine::PollStatus(HLVR_PlatformInfo* info)
{
	if (m_messenger.ConnectedToService(info)) {
	
			return HLVR_Ok;
		
	}

	return HLVR_Error_NotConnected;

	
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






void Engine::ReleaseHandle(unsigned int handle)
{
	m_player.Release(HapticHandle{ handle });
}



void Engine::HandlePause(unsigned int handle)
{
	m_player.Pause(HapticHandle{ handle });
}
void Engine::HandlePlay(unsigned int handle)
{
	m_player.Play(HapticHandle{ handle });
}
void Engine::HandleReset(unsigned int handle)
{
	m_player.Stop(HapticHandle{ handle });
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
		return HLVR_Error_NullArgument;
	}
	//enforces precondition on PlayableEffect to not have an empty effects list
	if (list->empty()) {
		return -1;
	}
	else {
		HapticHandle h = m_player.Create(extractPlayables(list->events()));
		*handle = h;
		return HLVR_Ok;
	}

	return HLVR_Error_Unknown;


}


void copyQuaternion(HLVR_Quaternion& lhs, const NullSpace::SharedMemory::Quaternion& rhs) {
	lhs.w = rhs.w;
	lhs.x = rhs.x;
	lhs.y = rhs.y;
	lhs.z = rhs.z;
}

void copyTracking(HLVR_TrackingUpdate& lhs, const NullSpace::SharedMemory::TrackingUpdate& rhs) {
	copyQuaternion(lhs.chest, rhs.chest);
	copyQuaternion(lhs.left_upper_arm, rhs.left_upper_arm);
	copyQuaternion(lhs.right_upper_arm, rhs.right_upper_arm);
}

int Engine::PollTracking(HLVR_TrackingUpdate* q)
{
	
	if (auto trackingUpdate = m_messenger.ReadTracking()) {
		copyTracking(*q, *trackingUpdate);
		return HLVR_Ok;
	}
	else {
		return HLVR_Ok_NoDataAvailable;
	}
}


//int Engine::PollLogs(NSVR_LogEntry * entry)
//{
//	if (auto logEntry = m_log->Poll()) {
//		auto str = *logEntry;
//		entry->Length = static_cast<uint32_t>(str.length());
//		strncpy_s(entry->Message, 512, str.c_str(), 512);
//		entry->Message[511] = '\0';
//		return NSVR_Success_Unqualified;
//	}
//	else {
//		return NSVR_Success_NoDataAvailable;
//	}
//}

//int Engine::EnableAudio(NSVR_AudioOptions* optionsPtr)
//{
//	NullSpaceIPC::DriverCommand command;
//	command.set_command(NullSpaceIPC::DriverCommand_Command_ENABLE_AUDIO);
//	if (optionsPtr == nullptr) {
//		(*command.mutable_params())["audio_min"] = 0x04;
//		(*command.mutable_params())["audio_max"] = 0x22;
//		(*command.mutable_params())["peak_time"] = 0x01;
//		(*command.mutable_params())["filter"] = 0x01;
//	}
//	else
//	{
//		(*command.mutable_params())["audio_min"] = std::min(std::max<int>(0, optionsPtr->AudioMin), 255);
//		(*command.mutable_params())["audio_max"] = std::min(std::max<int>(0, optionsPtr->AudioMax), 255);
//		(*command.mutable_params())["peak_time"] = std::min(std::max<int>(0, optionsPtr->PeakTime), 3);
//		(*command.mutable_params())["filter"] = std::min(std::max<int>(0, optionsPtr->Filter), 3);
//	}
//	m_messenger.WriteCommand(command);
//	return NSVR_Success_Unqualified;
//}
//
//int Engine::DisableAudio()
//{
//	NullSpaceIPC::DriverCommand command;
//	command.set_command(NullSpaceIPC::DriverCommand_Command_DISABLE_AUDIO);
//	m_messenger.WriteCommand(command);
//	return NSVR_Success_Unqualified;
//}
//
//int Engine::SubmitRawCommand(uint8_t * buffer, int length)
//{
//	NullSpaceIPC::DriverCommand command;
//	command.set_command(NullSpaceIPC::DriverCommand_Command_RAW_COMMAND);
//	command.set_raw_command(buffer, length);
//	m_messenger.WriteCommand(command);
//	return NSVR_Success_Unqualified;
//}
//
