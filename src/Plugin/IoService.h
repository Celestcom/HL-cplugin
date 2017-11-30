#pragma once
#include <boost\asio\io_service.hpp>
#include <boost\thread\condition_variable.hpp>
#include <boost\thread\barrier.hpp>
#include <atomic>
#include <thread>
class IoService
{
public:
	//Constructs and Start's a new IoService
	IoService();

	//Return the underlying io_service object
	boost::asio::io_service& GetIOService();


	//Tell it to shutdown
	void Shutdown();

private:

	void start();
	bool _running;
	bool _dataReady;

	std::thread _ioLoop;
	boost::asio::io_service _io;
	std::unique_ptr<boost::asio::io_service::work> _work;
	std::atomic_bool _shouldQuit;

};

