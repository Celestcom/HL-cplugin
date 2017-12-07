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

int Engine::GetInfo(uint32_t m_handle, HLVR_EffectInfo* infoPtr) const
{
	if (auto info = m_player.GetInfo(EffectHandle(m_handle))) {
		infoPtr->Duration = info->Duration;
		infoPtr->Elapsed = info->CurrentTime;
		infoPtr->PlaybackState = static_cast<HLVR_EffectInfo_State>(info->State);
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

int Engine::StreamEvent(const TypedEvent& event)
{
	auto ev = PlayableEvent::make(event.Type, 0.0f);
	if (!ev) {
		return HLVR_Error_InvalidEventType;
	}
	ev->parse(event.Params);

	NullSpaceIPC::HighLevelEvent hle;
	ev->serialize(hle);
	m_messenger.WriteEvent(hle);
	return HLVR_Ok;
}

int Engine::EnableTracking(uint32_t device_id)
{
	NullSpaceIPC::HighLevelEvent hle;
	auto device_event = hle.mutable_device_event();
	device_event->set_device(device_id);
	auto tracking_enable = device_event->mutable_enable_tracking();

	m_messenger.WriteEvent(hle);
	return 1;
}

int Engine::DisableTracking(uint32_t device_id)
{
	NullSpaceIPC::HighLevelEvent hle;
	auto device_event = hle.mutable_device_event();
	device_event->set_device(device_id);
	auto tracking_enable = device_event->mutable_disable_tracking();

	m_messenger.WriteEvent(hle);

	return 1;
}



Engine::Engine() :

	m_isHapticsSystemPlaying(true),
	m_ioService(),
	m_messenger(m_ioService.GetIOService()),
	m_player(m_ioService.GetIOService(), m_messenger),
	m_currentHandleId(0),
	m_hapticsExecutionInterval(boost::posix_time::milliseconds(5)),
	m_hapticsTimestep(m_ioService.GetIOService(), m_hapticsExecutionInterval),
	m_lastHapticsTimestep(),
	m_cachedTrackingUpdate({}),
	m_deviceSnapshots(),
	m_nodeSnapshots()
{

	

//	setupUserFacingLogSink();
	//setupFileLogSink(); //not implementing this yet


	boost::log::core::get()->set_logging_enabled(false);

	m_player.start();


}


void Engine::setupFileLogSink()
{
	using namespace boost::log;
	auto backend = boost::make_shared<sinks::text_file_backend>(
		  keywords::file_name = "nsvr_plugin_%5N.log"
		, keywords::rotation_size = 5 * 1024 * 1024
		, keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0)

	);


	using sink_t = sinks::synchronous_sink<sinks::text_file_backend>;
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
	
	using sink_t = sinks::synchronous_sink<MyTestLog>;
	boost::shared_ptr<sink_t> sink(new sink_t(m_log));
	core::get()->add_sink(sink);
	
}

Engine::~Engine()
{

	boost::log::core::get()->remove_all_sinks();

	try {
		m_player.ClearAll();
		m_player.stop();
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
		m_ioService.Shutdown();
	}
	catch (const std::exception&) {

	}
}

int Engine::PollStatus(HLVR_RuntimeInfo* info) const 
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
	default:
		break;
	}
	
	return true;
}






void Engine::ReleaseHandle(uint32_t handle)
{
	m_player.Release(handle);
}



int Engine::HandlePause(uint32_t handle)
{
	return m_player.Pause(handle);
}
int Engine::HandlePlay(uint32_t handle)
{
	return m_player.Play(handle);
}
int Engine::HandleReset(uint32_t handle)
{
	return m_player.Stop(handle);
}






std::vector<std::unique_ptr<PlayableEvent>> 
extractPlayables(const std::vector<TimeOffset<TypedEvent>>& events) {
	
	using PlayablePtr = std::unique_ptr<PlayableEvent>;
	std::vector<PlayablePtr> playables;
	playables.reserve(events.size());
	for (const auto& event : events) {
		if (auto newPlayable = PlayableEvent::make(event.Data.Type, event.Time)) {
			newPlayable->parse(event.Data.Params); 
			playables.push_back(std::move(newPlayable));
		}
	}

	return playables;
}

//Only modifies handle if the effect is created successfully
int Engine::CreateEffect(const EventList * list, EffectHandle* handle)
{
	if (list == nullptr) {
		return HLVR_Error_NullArgument;
	}
	//enforces precondition on PlayableEffect to not have an empty effects list
	if (list->empty()) {
		return HLVR_Error_EmptyTimeline;
	}
	else {
		EffectHandle h = m_player.Create(extractPlayables(list->events()));
		*handle = h;
		return HLVR_Ok;
	}

	return HLVR_Error_Unspecified;


}


inline void copyQuaternion(HLVR_Quaternion& lhs, const NullSpace::SharedMemory::Quaternion& rhs) {
	lhs.w = rhs.w;
	lhs.x = rhs.x;
	lhs.y = rhs.y;
	lhs.z = rhs.z;
}

inline void copyVector3f(HLVR_Vector3f& lhs, const NullSpace::SharedMemory::Vector3& rhs) {
	lhs.x = rhs.x;
	lhs.y = rhs.y;
	lhs.z = rhs.z;
}
void copyTracking(HLVR_TrackingUpdate& lhs, const NullSpace::SharedMemory::TrackingUpdate& rhs) {
	copyQuaternion(lhs.chest, rhs.chest);
	copyQuaternion(lhs.left_upper_arm, rhs.left_upper_arm);
	copyQuaternion(lhs.right_upper_arm, rhs.right_upper_arm);

	copyVector3f(lhs.chest_compass, rhs.chest_compass);
	copyVector3f(lhs.chest_gravity, rhs.chest_gravity);
	copyVector3f(lhs.left_upper_arm_compass, rhs.left_upper_arm_compass);
	copyVector3f(lhs.left_upper_arm_gravity, rhs.left_upper_arm_gravity);
	copyVector3f(lhs.right_upper_arm_compass, rhs.right_upper_arm_compass);
	copyVector3f(lhs.right_upper_arm_gravity, rhs.right_upper_arm_gravity);
}

int Engine::PollTracking(HLVR_TrackingUpdate* q)
{
	
	if (auto trackingUpdate = m_messenger.ReadTracking()) {
		copyTracking(*q, *trackingUpdate);
		return HLVR_Ok;
	}
	
	return HLVR_Ok_NoDataAvailable;
	
}


