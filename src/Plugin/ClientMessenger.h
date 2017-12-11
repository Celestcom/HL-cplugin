#pragma once

#include "ReadableSharedQueue.h"
#include "ReadableSharedMap.h"
#include "ReadableSharedObject.h"
#include "WritableSharedQueue.h"
#include "ReadableSharedVector.h"
#include "SharedTypes.h"
#include <boost\optional.hpp>
#include <boost\asio.hpp>
#include <boost\chrono.hpp>
#include <mutex>

#pragma warning(push)
#pragma warning(disable : 4267)
#include "HighLevelEvent.pb.h"
#pragma warning(pop)

typedef struct HLVR_RuntimeInfo HLVR_RuntimeInfo;
class ClientMessenger
{
public:
	ClientMessenger(boost::asio::io_service&);

	boost::optional<NullSpace::SharedMemory::TrackingUpdate> ReadTracking();

	boost::optional<NullSpace::SharedMemory::TrackingData> ClientMessenger::ReadTrackingData(uint32_t region);

	std::vector<NullSpace::SharedMemory::DeviceInfo> ReadDevices();
	std::vector<NullSpace::SharedMemory::NodeInfo> ReadNodes();

	//possible-thread-safe
	void WriteEvent(const NullSpaceIPC::HighLevelEvent& e);

	std::vector<NullSpace::SharedMemory::RegionPair> ReadBodyView();
	bool ConnectedToService(HLVR_RuntimeInfo* info) const;

	
private:
	NullSpace::SharedMemory::ServiceInfo m_serviceVersion;
	std::unique_ptr<WritableSharedQueue> m_events;
	std::mutex m_eventsLock;

	std::unique_ptr<ReadableSharedVector<NullSpace::SharedMemory::NodeInfo>> m_nodes;
	std::unique_ptr<ReadableSharedVector<NullSpace::SharedMemory::DeviceInfo>> m_devices;

	//Sentinel to see if the driver is running
	std::unique_ptr<ReadableSharedObject<NullSpace::SharedMemory::SentinelObject>> m_sentinel;

	std::unique_ptr<ReadableSharedVector<NullSpace::SharedMemory::TrackingData>> m_tracking;

	std::unique_ptr<ReadableSharedVector<NullSpace::SharedMemory::RegionPair>> m_bodyView;

	boost::asio::deadline_timer m_sentinelTimer;
	boost::posix_time::milliseconds m_sentinelCheckInterval;

	//If currentTime - sentinalTime > m_sentinalTimeout, we say that we are disconnected
	boost::chrono::milliseconds m_sentinalTimeout;

	void startAttemptEstablishConnection();

	void attemptEstablishConnection(const boost::system::error_code& ec);

	void startMonitorConnection();
	void monitorConnection(const boost::system::error_code& ec);

	bool m_connectedToService;
};

