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
	std::vector<NullSpace::SharedMemory::DeviceInfo> ReadDevices();
	std::vector<NullSpace::SharedMemory::NodeInfo> ReadNodes();
	void WriteEvent(const NullSpaceIPC::HighLevelEvent& e);
	boost::optional<std::string> ReadLog();

	std::vector<NullSpace::SharedMemory::RegionPair> ReadBodyView();
	bool ConnectedToService(HLVR_RuntimeInfo* info) const;

	
private:
	NullSpace::SharedMemory::ServiceInfo m_serviceVersion;
	//Write haptics to the suit using this shared queue
	std::unique_ptr<WritableSharedQueue> m_hapticsStream;


	std::unique_ptr<ReadableSharedVector<NullSpace::SharedMemory::NodeInfo>> m_nodes;
	std::unique_ptr<ReadableSharedVector<NullSpace::SharedMemory::DeviceInfo>> m_systems;
	//Read the most up-to-date suit connection information from this object
	// 
	//Get logging info from engine. Note: only one consumer can reliably get the debug info
	std::unique_ptr<ReadableSharedQueue> m_logStream;

	//Sentinel to see if the driver is running
	std::unique_ptr<ReadableSharedObject<NullSpace::SharedMemory::SentinelObject>> m_sentinel;

	//Stream of commands to send to driver, such as ENABLE_TRACKING, DISABLE_TRACKING, etc.
	std::unique_ptr<WritableSharedQueue> m_commandStream;

	std::unique_ptr<ReadableSharedVector<NullSpace::SharedMemory::TrackingData>> m_tracking;



	std::unique_ptr<ReadableSharedVector<NullSpace::SharedMemory::RegionPair>> m_bodyView;
	//We use a sentinel to see if the driver is responsive/exists
	boost::asio::deadline_timer m_sentinelTimer;

	//How often we read the sentinel
	boost::posix_time::milliseconds m_sentinelInterval;

	//If currentTime - sentinalTime > m_sentinalTimeout, we say that we are disconnected
	boost::chrono::milliseconds m_sentinalTimeout;



	void startAttemptEstablishConnection();

	void attemptEstablishConnection(const boost::system::error_code& ec);

	void startMonitorConnection();
	void monitorConnection(const boost::system::error_code& ec);

//	Encoder m_encoder;

	bool m_connectedToService;
};

