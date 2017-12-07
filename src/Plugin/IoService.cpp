#include "StdAfx.h"
#include "IoService.h"


IoService::IoService()
	: m_running{false}
	, m_ioLoop{}
	, m_io{}
	, m_work{}
	, m_shouldQuit{false}
{
	start();
}

void IoService::start() {
	m_running = true;
	m_ioLoop = std::thread([&]() {
		while (!m_shouldQuit.load()) {
			try {
				m_work = std::make_unique<boost::asio::io_service::work>(m_io);
				m_io.run(); //wait here for a while
				m_io.reset(); //someone stopped us? Reset
			}
			catch (boost::system::system_error&) {
				//The io_service couldn't reset?
				//todo: log this 
			}
		}
	});
}

void IoService::Shutdown()
{
	m_shouldQuit.store(true);
	m_work.reset();
	m_io.stop();
	if (m_ioLoop.joinable()) {
		m_ioLoop.join();
	}
}

boost::asio::io_service& IoService::GetIOService()
{
	return m_io;
}

