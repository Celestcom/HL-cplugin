#include "stdafx.h"
#include "RtpModel.h"



RtpModel::RtpModel(Location area) :m_area(area), m_volume(0), m_commands()
{
}

void RtpModel::ChangeVolume(int newVolume)
{
	if (newVolume != m_volume) {
		m_volume = newVolume;


		using namespace NullSpaceIPC;
		EffectCommand command;
		command.set_command(EffectCommand_Command::EffectCommand_Command_PLAY_RTP);
		command.set_strength((m_volume / 2) / 128.0f);
		command.set_area(static_cast<uint32_t>(m_area));

		std::lock_guard<std::mutex> guard(m_mutex);
		m_commands.push_back(std::move(command));

	}
}

CommandBuffer RtpModel::Update(float dt)
{
	CommandBuffer copy;
	std::lock_guard<std::mutex> guard(m_mutex);

	copy.swap(m_commands);
	std::reverse(copy.begin(), copy.end());
	return copy;

}

int RtpModel::GetVolume()
{
	return m_volume;
}