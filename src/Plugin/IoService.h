#pragma once
#include <boost\asio\io_service.hpp>
#include <boost\thread\condition_variable.hpp>
#include <boost\thread\barrier.hpp>
#include <atomic>
#include <thread>
class IoService
{
public:
	IoService();
	boost::asio::io_service& GetIOService();
	void Shutdown();
private:
	bool m_running;
	std::thread m_ioLoop;
	boost::asio::io_service m_io;
	std::unique_ptr<boost::asio::io_service::work> m_work;
	std::atomic_bool m_shouldQuit;

	void start();


};

