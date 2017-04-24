#include "stdafx.h"
#include "ClientMessenger.h"
//#include "Locator.h"
#include <boost\bind.hpp>
#include "EffectCommand.pb.h"

#include "NSLoader.h"

using namespace NullSpace::SharedMemory;
ClientMessenger::ClientMessenger(boost::asio::io_service& io):

	m_sentinelTimer(io),
	m_sentinelInterval(500),
	m_sentinalTimeout(2000),
	m_connectedToService(false)
{
	//First time we attempt to establish connection, do it with zero delay
	m_sentinelTimer.expires_from_now(boost::posix_time::millisec(0));
	m_sentinelTimer.async_wait([&](auto error) {attemptEstablishConnection(error); });
	static_assert(sizeof(std::time_t) == 8, "Time is wrong size");
}


ClientMessenger::~ClientMessenger()
{
}

boost::optional<TrackingUpdate> ClientMessenger::ReadTracking()
{
	if (m_trackingData) {
		return m_trackingData->Read();
	}
	return boost::optional<TrackingUpdate>();
}

boost::optional<SuitsConnectionInfo> ClientMessenger::ReadSuits()
{
	if (m_suitConnectionInfo) {
		return m_suitConnectionInfo->Read();
	}

	return boost::optional<SuitsConnectionInfo>();
}

void ClientMessenger::WriteCommand(const NullSpaceIPC::DriverCommand & d)
{
	std::string binaryData;
	d.SerializeToString(&binaryData);
	if (m_commandStream) {
		try {
			m_commandStream->Push(binaryData.data(), d.ByteSize());
		}
		catch (const boost::interprocess::interprocess_exception&) {
			//probably full queue
			//should log
		}
	}
}

void ClientMessenger::WriteHaptics(const NullSpaceIPC::EffectCommand& e)
{
	std::string binaryData;
	e.SerializeToString(&binaryData);
	if (m_hapticsStream) {
		try {
			m_hapticsStream->Push(binaryData.data(), e.ByteSize());
		}
		catch (const boost::interprocess::interprocess_exception&) {
			//probably full queue, which means the server isn't reading fast enough!
			//should log
		}
	}
	
}

boost::optional<std::string> ClientMessenger::ReadLog()
{
	if (m_logStream) {
		std::vector<unsigned char> chars = m_logStream->Pop();
		if (chars.empty()) {
			return boost::optional<std::string>();
		}
		std::string resultString(chars.begin(), chars.end());
		
		return resultString;
	}

	return boost::optional<std::string>();

}

bool ClientMessenger::ConnectedToService(NSVR_ServiceInfo* info)
{

	if (m_connectedToService) {
		if (info != nullptr) {
			info->ServiceMajor = 0;
			info->ServiceMinor = 0;
		}
		return true;
	}
	
	
	return false;
	
}


void ClientMessenger::startAttemptEstablishConnection()
{
	m_sentinelTimer.expires_from_now(m_sentinelInterval);
	m_sentinelTimer.async_wait(boost::bind(&ClientMessenger::attemptEstablishConnection, this, boost::asio::placeholders::error));
}

void ClientMessenger::attemptEstablishConnection(const boost::system::error_code &)
{
	try {
		m_sentinel = std::make_unique<ReadableSharedObject<std::time_t>>("ns-sentinel");

	}
	catch (const boost::interprocess::interprocess_exception&) {

		//the shared memory object doesn't exist yet? Try again
		startAttemptEstablishConnection();
		return;
	}


	//Once the sentinel has connected, we want to setup the other shared objects
	try {
		//Locator::Logger().Log("ClientMessenger", "Attempting to create all the other shared objects");

		m_hapticsStream = std::make_unique<WritableSharedQueue>("ns-haptics-data");
		m_trackingData = std::make_unique<ReadableSharedObject<TrackingUpdate>>("ns-tracking-data");
		m_suitConnectionInfo = std::make_unique<ReadableSharedObject<SuitsConnectionInfo>>("ns-suit-data");
		m_commandStream = std::make_unique<WritableSharedQueue>("ns-command-data");
	}
	catch (const boost::interprocess::interprocess_exception& e) {
		BOOST_LOG_TRIVIAL(error) << "Failed to make shared objects: " << e.what();
		//Locator::Logger().Log("ClientMessenger", "Failed to create all the other shared objects", LogLevel::Error);

		//somehow failed to make these shared objects.
		//for now, until we know what types of errors these are, try again
		startAttemptEstablishConnection();
		return;
	}
	try {
		m_logStream = std::make_unique<ReadableSharedQueue>("ns-logging-data");

	
		
	}
	catch (const boost::interprocess::interprocess_exception& e) {
		BOOST_LOG_TRIVIAL(error) << "Failed to make shared objects: " << e.what();
		//Locator::Logger().Log("ClientMessenger", "Failed to create all the other shared objects", LogLevel::Error);

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
	m_sentinelTimer.expires_from_now(m_sentinelInterval);
	m_sentinelTimer.async_wait([&](auto error) { monitorConnection(error); });
}

void ClientMessenger::monitorConnection(const boost::system::error_code & ec)
{
	if (!ec) {
	//	Locator::Logger().Log("ClientMessenger", "Reading the sentinal..");

		std::time_t lastDriverTimestamp = m_sentinel->Read();
		//assumes that the current time is >= the read time
		auto time = boost::chrono::duration_cast<boost::chrono::milliseconds>(
			boost::chrono::seconds(std::time(nullptr) - lastDriverTimestamp)
		);

		if (time <= m_sentinalTimeout) {
			m_connectedToService = true;
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


