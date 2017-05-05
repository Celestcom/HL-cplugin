#pragma once
#include <boost/uuid/uuid.hpp>
#include "NSLoader_fwds.h"
#include "SuitEvent.h"
#include "HapticQueue.h"
class RealtimeArgs {

};

//should be renamed to just Driver or HardwareDriver or something
class HardwareDriver {
public:
	
	virtual void createRetained(boost::uuids::uuid handle, const SuitEvent& event) {}
	virtual void controlRetained(boost::uuids::uuid handle, NSVR_PlaybackCommand command) {}
	virtual void realtime(const RealtimeArgs& args) {}
	
};

