#include "stdafx.h"
#include "MotorStateChanger.h"
#include "HardlightMk3Firmware.h"

MotorStateChanger::MotorStateChanger(Location areaId) :
	currentState(MotorFirmwareState::Idle),
	previousContinuous(),
	area(areaId)
{
}

MotorStateChanger::MotorFirmwareState MotorStateChanger::GetState() const
{
	return currentState;
}

CommandBuffer MotorStateChanger::transitionTo(const LiveBasicHapticEvent & event)
{
	CommandBuffer commands;
	if (event == previousContinuous) {
		commands = CommandBuffer();
	}
	else if (event.isOneshot()) {

		commands = transitionToOneshot(event.Data());
		previousContinuous = boost::optional<LiveBasicHapticEvent>();

	}
	else {
		commands = transitionToContinuous(event.Data());
		previousContinuous = event;

	}

	return commands;
}

CommandBuffer MotorStateChanger::transitionToIdle()
{
	CommandBuffer requiredCmds;
	switch (currentState) {
	case MotorFirmwareState::Idle:
		//do nothing
		break;
	case MotorFirmwareState::PlayingOneshot:
		//do nothing;
		break;
	case MotorFirmwareState::PlayingContinuous:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateHalt(area));
		break;
	}

	currentState = MotorFirmwareState::Idle;
	previousContinuous = boost::optional<LiveBasicHapticEvent>();
	return requiredCmds;
}

CommandBuffer MotorStateChanger::transitionToOneshot(BasicHapticEventData data)
{
	//Note: as you can see, we generate the same commands from every state.
	//This is subject to change with future firmware versions, which is why it is 
	//setup like this.

	CommandBuffer requiredCmds;
	switch (currentState) {
	case MotorFirmwareState::Idle:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateHalt(area));
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateOneshotPlay(data));
		break;
	case MotorFirmwareState::PlayingOneshot:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateHalt(area));
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateOneshotPlay(data));
		break;
	case MotorFirmwareState::PlayingContinuous:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateHalt(area));
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateOneshotPlay(data));
		break;
	}

	currentState = MotorFirmwareState::PlayingOneshot;
	return requiredCmds;
}

CommandBuffer MotorStateChanger::transitionToContinuous(BasicHapticEventData data)
{
	//Note: as you can see, we generate the same commands from every state.
	//This is subject to change with future firmware versions, which is why it is 
	//setup like this.

	CommandBuffer requiredCmds;
	switch (currentState) {
	case MotorFirmwareState::Idle:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateContinuousPlay(data));
		break;
	case MotorFirmwareState::PlayingOneshot:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateContinuousPlay(data));
		break;
	case MotorFirmwareState::PlayingContinuous:
		requiredCmds.push_back(Hardlight_Mk3_Firmware::generateContinuousPlay(data));
		break;
	}

	currentState = MotorFirmwareState::PlayingContinuous;
	return requiredCmds;
}
