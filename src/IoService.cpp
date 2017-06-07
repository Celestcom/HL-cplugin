#include "StdAfx.h"
#include "IoService.h"
#include <iostream>


//To those who would venture into this code:
//It is very old. With a good testing suite, we may be able to replace it with something less complex
//now that most of the serial issues are moved into the driver. 

IoService::IoService():
	_io(), 
	_work(), 
	_running(false), 
	_shouldQuit{false}
{
	start();
}

void IoService::start() {
	if (_running) {
		return;
	}
	_running = true;
	
	_ioLoop = std::thread([&]() {
		
		while (!_shouldQuit.load()) {
			try {
				_work = std::make_unique<boost::asio::io_service::work>(_io);

				_io.run(); //wait here for a while
				_io.reset(); //someone stopped us? Reset

			}
			catch (boost::system::system_error&) {
				//The Io service couldn't reset?!
				//todo: log this 
			}

		}
	});
	
	
}
void IoService::Shutdown()
{
	
	_shouldQuit.store(true);
	_io.stop();

	if (_ioLoop.joinable()) {
		_ioLoop.join();
	}

}

IoService::~IoService()
{
	
}

boost::asio::io_service& IoService::GetIOService()
{
	return _io;
}

