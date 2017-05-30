#pragma once

#include "Enums.h"
#include "IHapticDevice.h"
#include <mutex>
#include <boost/optional/optional.hpp>
#include "EffectCommand.pb.h"

class RtpModel {
public:
	RtpModel(Location area);
	void ChangeVolume(int newVolume);
	int GetVolume();
	CommandBuffer Update(float dt);
private:
	int volume;
	Location location;

	boost::optional<NullSpaceIPC::EffectCommand> volumeCommand;
	std::mutex volumeValueProtector;
};