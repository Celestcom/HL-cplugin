#include "stdafx.h"
#include "RtpModel.h"



RtpModel::RtpModel(Location area) : 
	location(area), 
	volume(0), 
	volumeValueProtector(),
	volumeCommand(boost::none)
{
}

void RtpModel::ChangeVolume(int newVolume)
{
	using namespace NullSpaceIPC;

	std::lock_guard<std::mutex> guard(volumeValueProtector);

	if (newVolume != volume) {
		volume = newVolume;
		EffectCommand command;
		command.set_command(EffectCommand_Command::EffectCommand_Command_PLAY_RTP);
		command.set_strength((volume / 2) / 128.0f);
		command.set_area(static_cast<uint32_t>(location));
		volumeCommand = command;
	}
	else {
		volumeCommand = boost::none;
	}

}

CommandBuffer RtpModel::Update(float dt)
{
	CommandBuffer commands;

	std::lock_guard<std::mutex> guard(volumeValueProtector);

	if (volumeCommand) {
		commands.push_back(std::move(*volumeCommand));
		volumeCommand = boost::none;
	}

	return commands;
}

int RtpModel::GetVolume()
{
	return volume;
}