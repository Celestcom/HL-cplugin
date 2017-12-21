#pragma once

// ClientMessenger's purpose is to communicate with the runtime. Internally
// it uses a shared memory bridge, essentially a bunch of objects/queues/vectors.
// Note that it is much less error-prone to use simple structures when possible

#include "ReadableSharedQueue.h"
#include "ReadableSharedMap.h"
#include "ReadableSharedObject.h"
#include "WritableSharedQueue.h"
#include "ReadableSharedVector.h"
#include "SharedTypes.h"
#include "HLVR_Errors.h"
#include <boost\optional.hpp>
#include <boost\asio.hpp>
#include <boost\chrono.hpp>
#include "expected.hpp"
#include <mutex>

#pragma warning(push)
#pragma warning(disable : 4267)
#include "HighLevelEvent.pb.h"
#pragma warning(pop)

typedef struct HLVR_RuntimeInfo HLVR_RuntimeInfo;


class ClientMessenger
{
public:
	//The io_service reference must remain valid for as long as ClientMessenger lives
	ClientMessenger(boost::asio::io_service&);

	tl::expected<NullSpace::SharedMemory::TrackingData, HLVR_Result> ClientMessenger::ReadTrackingData(uint32_t region);

	tl::expected<std::vector<NullSpace::SharedMemory::DeviceInfo>, HLVR_Result> ReadDevices() const;
	tl::expected<std::vector<NullSpace::SharedMemory::NodeInfo>, HLVR_Result> ReadNodes() const;

	void WriteEvent(const NullSpaceIPC::HighLevelEvent& e);

	tl::expected<std::vector<NullSpace::SharedMemory::RegionPair>, HLVR_Result> ReadBodyView() const;
	int ConnectedToService(HLVR_RuntimeInfo* info) const;

	
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

	bool compatibleService();
	void identifyClient();
	
	boost::optional<bool> m_connectedToService;
};

