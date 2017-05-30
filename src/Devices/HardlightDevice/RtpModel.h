#pragma once

#include "Enums.h"
#include "IHapticDevice.h"
#include <mutex>


class RtpModel {
public:
	
	RtpModel(Location area);
	void ChangeVolume(int newVolume);
	CommandBuffer Update(float dt);
	int GetVolume();
private:
	int m_volume;
	Location m_area;
	CommandBuffer m_commands;
	std::mutex m_mutex;
};