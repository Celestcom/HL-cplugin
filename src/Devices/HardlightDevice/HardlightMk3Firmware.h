#pragma once
#include "EffectCommand.pb.h"
#include "LiveBasicHapticEvent.h"
#include "Enums.h"
//Todo: This is not good design. The plugin shouldn't have to know the specific firmware commands.
//We should push this knowledge into the Driver so that the plugin can issue PLAY_DURATION and it will issue
//HALT - PLAY_CONT - or whatever is actually necessary to make the hardware respond correctly.
class Hardlight_Mk3_Firmware {
public:
	static NullSpaceIPC::EffectCommand generateContinuousPlay(Location area, const BasicHapticEventData& data);
	static NullSpaceIPC::EffectCommand generateOneshotPlay(Location area, const BasicHapticEventData& data);
	static NullSpaceIPC::EffectCommand generateHalt(Location area);
};

