#include "stdafx.h"
#include "ClientMessenger.h"
//#include "Locator.h"
#include "HLVR.h"
#include <boost\bind.hpp>
#include <boost/log/trivial.hpp>

#pragma warning(push)
#pragma warning(disable: 4267)
#include "HighLevelEvent.pb.h"
#pragma warning(pop)

#include "HLVR.h"

#include "BoostIPCSharedMemoryDirectory.h"
using namespace NullSpace::SharedMemory;

constexpr int MIN_COMPATIBLE_SERVICE_VERSION_MAJOR = 1;
constexpr int MIN_COMPATIBLE_SERVICE_VERSION_MINOR = 0;
constexpr int MIN_COMPATIBLE_SERVICE_VERSION_PATCH = 5;
ClientMessenger::ClientMessenger(boost::asio::io_service& io):
	m_serviceVersion(),
	m_sentinelTimer(io),
	m_sentinelCheckInterval(500),
	m_sentinalTimeout(2000),
	m_connectedToService(),
	m_events(),
	m_eventsLock(),
	m_devices(),
	m_nodes(),
	m_tracking(),
	m_bodyView()
{
	//First time we attempt to establish connection, do it with zero delay
	m_sentinelTimer.expires_from_now(boost::posix_time::millisec(0));
	m_sentinelTimer.async_wait([&](auto error) {attemptEstablishConnection(error); });
	static_assert(sizeof(std::time_t) == 8, "Time is wrong size");
}


tl::expected<NullSpace::SharedMemory::TrackingData, HLVR_Result> ClientMessenger::ReadTrackingData(uint32_t region) {
	if (!m_tracking) {
		return tl::make_unexpected(HLVR_Error_ServiceNotConnected);
	}
	if (m_tracking->Size() > 0) {
		if (auto val = m_tracking->Get([region](const auto& taggedQuat) { return taggedQuat.region == region; })) {
			return *val;
		}
	} 
	return tl::make_unexpected(HLVR_Error_TrackedRegionNotFound);
}

boost::optional<TrackingUpdate> ClientMessenger::ReadTracking()
{
	//if (m_trackingData) {
	//	return m_trackingData->Read();
	//}
	//return boost::optional<TrackingUpdate>();

	if (m_tracking) {
		if (m_tracking->Size() > 0) {
			TrackingUpdate t = {};

			if (auto val = m_tracking->Get([](const auto& taggedQuat) { return taggedQuat.region == hlvr_region_middle_sternum; })) {
				t.chest = val->quat;
				t.chest_compass = val->compass;
				t.chest_gravity = val->gravity;
			}
			if (auto val = m_tracking->Get([](const auto& taggedQuat) { return taggedQuat.region == hlvr_region_upper_arm_left; })) {
				t.left_upper_arm = val->quat;
				t.left_upper_arm_compass = val->compass;
				t.left_upper_arm_gravity = val->gravity;
			}
			if (auto val = m_tracking->Get([](const auto& taggedQuat) { return taggedQuat.region == hlvr_region_upper_arm_right; })) {
				t.right_upper_arm = val->quat;
				t.right_upper_arm_compass = val->compass;
				t.right_upper_arm_gravity = val->gravity;
			}

			return t;
		}
	}
	
	return boost::none;
	


}

std::vector<NullSpace::SharedMemory::DeviceInfo> ClientMessenger::ReadDevices()
{
	std::vector<NullSpace::SharedMemory::DeviceInfo> info;
	if (m_devices) {
		info = m_devices->ToVector();
	}
	return info;

}

std::vector<NullSpace::SharedMemory::NodeInfo> ClientMessenger::ReadNodes()
{
	std::vector<NullSpace::SharedMemory::NodeInfo> info;
	if (m_nodes) {
		info = m_nodes->ToVector();
	}
	return info;
}



void ClientMessenger::WriteEvent(const NullSpaceIPC::HighLevelEvent & e)
{
	std::string binaryData;
	e.SerializeToString(&binaryData);
	if (m_events) {
		try {
			std::lock_guard<std::mutex> guard{ m_eventsLock };
			m_events->Push(binaryData.data(), e.ByteSize());
		}
		catch (const boost::interprocess::interprocess_exception& e) {
			BOOST_LOG_TRIVIAL(warning) << "[ClientMessenger] Unable to push to haptics stream! " << e.what();
		}
	}
}



std::vector<NullSpace::SharedMemory::RegionPair> ClientMessenger::ReadBodyView()
{

	std::vector<NullSpace::SharedMemory::RegionPair> pairs;
	
	if (m_bodyView) {
		pairs = m_bodyView->ToVector();
	}
	
	return pairs;
}

int ClientMessenger::ConnectedToService(HLVR_RuntimeInfo* info) const
{

	if (m_connectedToService) {
		//okay, compatible service
		if (*m_connectedToService) {
			//okay, actually connected
			if (info != nullptr) {
				info->MajorVersion = m_serviceVersion.MajorVersion;
				info->MinorVersion = m_serviceVersion.MinorVersion;
				info->PatchVersion = m_serviceVersion.PatchVersion;
			}
			return HLVR_Ok;
		}
	
		return HLVR_Error_ServiceNotConnected;	
	}
	
	return HLVR_Error_ServiceIncompatible;	
}


void ClientMessenger::startAttemptEstablishConnection()
{
	m_sentinelTimer.expires_from_now(m_sentinelCheckInterval);
	m_sentinelTimer.async_wait(boost::bind(&ClientMessenger::attemptEstablishConnection, this, boost::asio::placeholders::error));
}

void ClientMessenger::attemptEstablishConnection(const boost::system::error_code &)
{
	//Step 1: read the sentinel object. This object should ideally never be changed, so that all services and all clients
	//can always read from it. 
	try {
		m_sentinel = std::make_unique<ReadableSharedObject<NullSpace::SharedMemory::SentinelObject>>("ns-sentinel");
		m_serviceVersion = m_sentinel->Read().Info;
		if (compatibleService()) {
			m_connectedToService = false;
		}
		else {
			m_connectedToService = boost::none;
		}
	}
	catch (const boost::interprocess::interprocess_exception&) {
		startAttemptEstablishConnection();
		return;
	}

	//Step 2: read the events queue. This may be changed, but it should remain compatible for at least a while. 
	//If we can open it, we identify ourselves to the service.
	try {
		m_events = std::make_unique<WritableSharedQueue>("ns-haptics-data");
		identifyClient();
		//here is the service's chance to throw up a "Old/newer client was connected" message
	}
	catch (const boost::interprocess::interprocess_exception&) {
		startAttemptEstablishConnection();
		return;
	}


	try {
		static_assert(sizeof(char) == 1, "set char size to 1");
		m_devices = std::make_unique<ReadableSharedVector<NullSpace::SharedMemory::DeviceInfo>>("ns-device-mem", "ns-device-data");
		m_nodes = std::make_unique<ReadableSharedVector<NullSpace::SharedMemory::NodeInfo>>("ns-node-mem", "ns-node-data");
		m_tracking = std::make_unique<ReadableSharedVector<NullSpace::SharedMemory::TrackingData>>("ns-tracking-mem", "ns-tracking-data");
		m_bodyView = std::make_unique<ReadableSharedVector<NullSpace::SharedMemory::RegionPair>>("ns-bodyview-mem", "ns-bodyview-data");

	}
	catch (const boost::interprocess::interprocess_exception& e) {
		BOOST_LOG_TRIVIAL(error) << "Failed to make shared objects: " << e.what();
	
		//somehow failed to make these shared objects.
		//for now, until we know what types of errors these are, try again
		startAttemptEstablishConnection();
		return;
	}
	

	//Everything setup successfully? Monitor the connection!
	startMonitorConnection();

	
}

void ClientMessenger::startMonitorConnection()
{
	m_sentinelTimer.expires_from_now(m_sentinelCheckInterval);
	m_sentinelTimer.async_wait([&](auto error) { monitorConnection(error); });
}

void ClientMessenger::monitorConnection(const boost::system::error_code & ec)
{
	if (!ec) {
	//	Locator::Logger().Log("ClientMessenger", "Reading the sentinal..");
		auto info = m_sentinel->Read();
		std::time_t lastDriverTimestamp = info.TimeStamp;
	
		//assumes that the current time is >= the read time
		auto time = boost::chrono::duration_cast<boost::chrono::milliseconds>(
			boost::chrono::seconds(std::time(nullptr) - lastDriverTimestamp)
		);

		if (time <= m_sentinalTimeout) {
			m_connectedToService = true;
			m_serviceVersion = info.Info;
			//we are connected, so keep monitoring
			//Locator::Logger().Log("ClientMessenger", "All good!");

			startMonitorConnection();
		}
		else {
			m_connectedToService = false;
			startAttemptEstablishConnection();
		}
	}
	else {
		//Locator::Logger().Log("ClientMessenger", "Monitor connection was cancelled");
	}

}


bool ClientMessenger::compatibleService()
{
	if (m_serviceVersion.MajorVersion < MIN_COMPATIBLE_SERVICE_VERSION_MAJOR) { return false; }
	if (m_serviceVersion.MajorVersion > MIN_COMPATIBLE_SERVICE_VERSION_MAJOR) { return true; }
	if (m_serviceVersion.MinorVersion < MIN_COMPATIBLE_SERVICE_VERSION_MINOR) { return false; }
	if (m_serviceVersion.MinorVersion > MIN_COMPATIBLE_SERVICE_VERSION_MINOR) { return true; }
	if (m_serviceVersion.PatchVersion < MIN_COMPATIBLE_SERVICE_VERSION_PATCH) { return false; }
	return true;
}

void ClientMessenger::identifyClient()
{
	NullSpaceIPC::HighLevelEvent event;
	auto client_id_event = event.mutable_client_id_event();
	client_id_event->set_dll_major(HLVR_API_VERSION_MAJOR);
	client_id_event->set_dll_minor(HLVR_API_VERSION_MINOR);
	client_id_event->set_dll_patch(HLVR_API_VERSION_PATCH);
	WriteEvent(event);
}


