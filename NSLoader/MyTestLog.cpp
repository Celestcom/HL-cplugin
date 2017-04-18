#include "stdafx.h"
#include "MyTestLog.h"
#include <boost\log\expressions.hpp>




MyTestLog::MyTestLog()
{
}

MyTestLog::~MyTestLog()
{
	m_readDriverLogs->Stop();
}

void MyTestLog::consume(const boost::log::record_view& msg)
{
	addEntry(*msg[boost::log::expressions::smessage]);
}

//this is basically the constructor
void MyTestLog::Provide(ClientMessenger* messenger, boost::asio::io_service& io)
{
	m_messenger = messenger;
	m_readDriverLogs = std::make_unique<ScheduledEvent>(io, boost::posix_time::milliseconds(50));
	m_readDriverLogs->SetEvent([this]() {

		auto optionalLog = m_messenger->ReadLog();
		if (optionalLog) {
			addEntry(*optionalLog);
		}

	});

	m_readDriverLogs->Start();
}

boost::optional<std::string> MyTestLog::Poll()
{
	if (m_log.empty()) {
		return boost::optional<std::string>();
	}

	std::string result = m_log.front();
	m_log.pop_front();
	return result;
}

void MyTestLog::addEntry(std::string entry)
{
	std::lock_guard<std::mutex> guard(m_logLock);
	if (m_log.size() > 256) {
		m_log.pop_front();
	}

	m_log.push_back(entry);
}
