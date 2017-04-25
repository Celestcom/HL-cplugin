#pragma once
#include <boost\log\core.hpp>
#include <boost\log\sinks\basic_sink_backend.hpp>
#include <boost\log\sinks\frontend_requirements.hpp>
#include "ClientMessenger.h"
#include <boost/asio/io_service.hpp>
#include <deque>
#include <memory>
#include "ScheduledEvent.h"

class MyTestLog : public boost::log::sinks::basic_sink_backend<
	boost::log::sinks::combine_requirements<
		boost::log::sinks::synchronized_feeding
	>::type>
{
public:
	MyTestLog();
	~MyTestLog();

	void consume(const boost::log::record_view& msg);

	void Provide(ClientMessenger* messenger, boost::asio::io_service& io);

	boost::optional<std::string> Poll();
private:
	ClientMessenger* m_messenger;
	std::deque<std::string> m_log;
	std::unique_ptr<ScheduledEvent> m_readDriverLogs;
	std::mutex m_logLock;

	void addEntry(std::string entry);
};

