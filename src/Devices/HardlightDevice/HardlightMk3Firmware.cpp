#include "stdafx.h"
#include "HardlightMk3Firmware.h"


NullSpaceIPC::EffectCommand Hardlight_Mk3_Firmware::generateContinuousPlay(const BasicHapticEventData & data)
{
	using namespace NullSpaceIPC;
	EffectCommand command;
	command.set_area(data.area);
	command.set_command(NullSpaceIPC::EffectCommand_Command_PLAY_CONTINUOUS);
	command.set_effect(data.effect);
	command.set_strength(data.strength);

	return command;
}

NullSpaceIPC::EffectCommand Hardlight_Mk3_Firmware::generateOneshotPlay(const BasicHapticEventData & data)
{
	using namespace NullSpaceIPC;
	EffectCommand command;
	command.set_area(data.area);
	command.set_command(NullSpaceIPC::EffectCommand_Command_PLAY);
	command.set_effect(data.effect);
	command.set_strength(data.strength);

	return command;
}

NullSpaceIPC::EffectCommand Hardlight_Mk3_Firmware::generateHalt(Location area)
{
	using namespace NullSpaceIPC;
	EffectCommand command;
	command.set_area(static_cast<uint32_t>(area));
	command.set_command(NullSpaceIPC::EffectCommand_Command_HALT);
	return command;
}
