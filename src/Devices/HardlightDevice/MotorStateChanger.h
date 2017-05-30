#pragma once
#include "Enums.h"
#include "LiveBasicHapticEvent.h"
#include "IHapticDevice.h"
#include <boost/optional.hpp>
class MotorStateChanger {
public:
	MotorStateChanger(Location areaId);
	enum class MotorFirmwareState {Idle, PlayingOneshot, PlayingContinuous};
	MotorFirmwareState GetState() const;
	CommandBuffer transitionTo(const LiveBasicHapticEvent& event);
	CommandBuffer transitionToIdle();
private:
	MotorFirmwareState currentState;
	boost::optional<LiveBasicHapticEvent> previousContinuous;
	Location area;
	CommandBuffer transitionToOneshot(BasicHapticEventData data);
	CommandBuffer transitionToContinuous(BasicHapticEventData data);
};
