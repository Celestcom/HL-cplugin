#pragma once
#include <boost/uuid/uuid.hpp>
#include "NSLoader_fwds.h"
#include "SuitEvent.h"
#include <boost/uuid/uuid.hpp>
class RealtimeArgs {
public:
	int volume;
};

class HardwareDriver {
public:
	
	virtual void createRetained(boost::uuids::uuid handle, const SuitEvent& event) {}
	virtual void controlRetained(boost::uuids::uuid handle, NSVR_PlaybackCommand command) {}
	virtual void realtime(const RealtimeArgs& args) {}
	virtual boost::uuids::uuid Id() const = 0;
};

bool operator<(const HardwareDriver& lhs, const HardwareDriver& rhs);