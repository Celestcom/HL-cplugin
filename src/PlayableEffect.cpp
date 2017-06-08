#include "stdafx.h"
#include "PlayableEffect.h"
#include "Locator.h"
#include <iostream>
#include "BasicHapticEvent.h"
#include "PriorityModel.h"
#include "NSLoader.h"
#include <memory>
#include <iterator>

namespace NS {
	namespace Playable {
		void Restart(IPlayable& playable) {
			playable.Stop();
			playable.Play();
		}
	}
}



PlayableEffect::PlayableEffect(std::vector<PlayablePtr>&& effects,EventRegistry& reg, boost::uuids::random_generator& uuid) :
	m_effects(std::move(effects)),
	m_state(PlaybackState::IDLE),
	m_registry(reg),
	m_id(uuid()),
	m_time(0)
{
	assert(!m_effects.empty());

	std::sort(m_effects.begin(), m_effects.end());

	reset();
}



PlayableEffect::~PlayableEffect()
{
	//shouldn't have to do this anymore if you call Stop first
	//if necessary, we go through activeConsumers and stop everything
}

void PlayableEffect::Play()
{
	switch (m_state) {
	case PlaybackState::IDLE:
		m_state = PlaybackState::PLAYING;
		break;
	case PlaybackState::PAUSED:
		resume();
		m_state = PlaybackState::PLAYING;
		break;
	case PlaybackState::PLAYING:
		//remain in playing state
		break;
	default:
		break;
	}
	
}

void PlayableEffect::Stop()
{

	switch (m_state) {
		case PlaybackState::IDLE:
			//remain in idle state
			break;
		case PlaybackState::PAUSED:
			reset();
			m_state = PlaybackState::IDLE;
			break;
		case PlaybackState::PLAYING:
			reset();
			m_state = PlaybackState::IDLE;
			break;
		default:
			break;
	}
}

void PlayableEffect::Pause()
{
	switch (m_state) {
	case PlaybackState::IDLE:
		//remain in idle state
		break;
	case PlaybackState::PAUSED:
		//remain in paused state
		break;
	case PlaybackState::PLAYING:
		pause();
		m_state = PlaybackState::PAUSED;
		break;
	default:
		break;
	}
	
	
}


void PlayableEffect::Update(float dt)
{
	if (m_state == PlaybackState::IDLE || m_state == PlaybackState::PAUSED) {
		return;
	}

	m_time += dt;
	
	auto current(m_lastExecutedEffect);

	auto isTimeExpired = [this](const PlayablePtr& event) {
		return event->time() <= m_time;
	};

	
	while (current != m_effects.end()) {
		if (isTimeExpired(*current)) {
			std::vector<std::string> regions = extractRegions(*current);
			for (const auto& region : regions) {
				auto consumers = m_registry.GetEventDrivers(region); 
				if (consumers) {
					//need translator from registry's leaves to area flags for backwards compat?
					std::for_each(consumers->begin(), consumers->end(), [&](auto& consumer) {

						consumer->createRetained(m_id, *current);
						m_activeDrivers.insert(std::weak_ptr<HardwareDriver>(consumer));
					});
				}
			}
			std::advance(current, 1);
		}
		else {
			//precondition: this requires that the effects vector was sorted by time.
			//Given that, we can stop here because if it wasn't expired, then the next won't be either
			break;
		}
	}

	m_lastExecutedEffect = current;

	if (m_time >= GetTotalPlayTime()) {
		Stop();
	}

} 



float PlayableEffect::GetTotalPlayTime() const
{
	TotalPlaytimeVisitor playtimeCounter;
	std::for_each(m_effects.begin(), m_effects.end(), boost::apply_visitor(playtimeCounter));
	return playtimeCounter.TotalPlaytime();

}

float PlayableEffect::CurrentTime() const
{
	return m_time;
}

bool PlayableEffect::IsPlaying() const
{
	return m_state == PlaybackState::PLAYING;
}

PlayableInfo PlayableEffect::GetInfo() const
{
	return PlayableInfo(GetTotalPlayTime(), m_time, m_state == PlaybackState::PLAYING);
}

void PlayableEffect::Release()
{
	std::for_each(m_activeDrivers.begin(), m_activeDrivers.end(), [&](std::weak_ptr<HardwareDriver> hd) {
		if (auto p = hd.lock()) {
			p->controlRetained(m_id, NSVR_PlaybackCommand::NSVR_PlaybackCommand_Reset);
		} else  {
			//the driver was removed, probably physically, so we don't need to worry about it
		}
	});
}

void PlayableEffect::reset()
{
	m_time = 0;
	m_lastExecutedEffect = m_effects.begin();
	

	std::for_each(m_activeDrivers.begin(), m_activeDrivers.end(), [&](std::weak_ptr<HardwareDriver> hd) {
		if (auto p = hd.lock()){
			p->controlRetained(m_id, NSVR_PlaybackCommand::NSVR_PlaybackCommand_Reset);
		}
		else {
			//the driver was removed, probably physically, so we don't need to worry about it
		}
	});
}

void PlayableEffect::pause()
{
	std::for_each(m_activeDrivers.begin(), m_activeDrivers.end(), [&](std::weak_ptr<HardwareDriver> hd) {
		if (auto p = hd.lock()) {
			p->controlRetained(m_id, NSVR_PlaybackCommand::NSVR_PlaybackCommand_Pause);
		}
		else {
			//the driver was removed, probably physically, so we don't need to worry about it
		}
	});
}

void PlayableEffect::resume() {
	//Now here's an interesting case. What if they unplug and switch suits with an effect paused? We want to resume on the new hardware right?
	//So if we fail to obtain the ptr, we should probably re-try to put this on a new set of drivers! But not now.

	std::for_each(m_activeDrivers.begin(), m_activeDrivers.end(), [&](std::weak_ptr<HardwareDriver> hd) {
		if (auto p = hd.lock()) {
			p->controlRetained(m_id, NSVR_PlaybackCommand::NSVR_PlaybackCommand_Play);
		}
		else {
			//the driver was removed, probably physically, so we don't need to worry about it
		}
	});
}



EventVisitor::EventVisitor(float time):m_time(time)
{

}

TotalPlaytimeVisitor::TotalPlaytimeVisitor():
	m_totalPlaytime(0), 
	m_fudgeFactor(0.25f)
{
}

float TotalPlaytimeVisitor::TotalPlaytime()
{
	return m_totalPlaytime;
}

RegionVisitor::RegionVisitor()
{
}
#define START_BITMASK_SWITCH(x) \
for (uint32_t bit = 1; x >= bit; bit *=2) if (x & bit) switch(AreaFlag(bit))

std::vector<std::string> extractRegions(const PlayablePtr & event) 
{
	auto translator = Locator::getTranslator();
	std::vector<std::string> regions;
	uint32_t area = event->area();
	START_BITMASK_SWITCH(area) {
		case AreaFlag::Forearm_Left:
			regions.push_back(translator.ToRegionString(AreaFlag::Forearm_Left));
			break;
		case AreaFlag::Upper_Arm_Left:
			regions.push_back(translator.ToRegionString(AreaFlag::Upper_Arm_Left));
			break;
		case AreaFlag::Shoulder_Left:
			regions.push_back(translator.ToRegionString(AreaFlag::Shoulder_Left));
			break;
		case AreaFlag::Back_Left:
			regions.push_back(translator.ToRegionString(AreaFlag::Back_Left));
			break;
		case AreaFlag::Chest_Left:
			regions.push_back(translator.ToRegionString(AreaFlag::Chest_Left));
			break;
		case AreaFlag::Upper_Ab_Left:
			regions.push_back(translator.ToRegionString(AreaFlag::Upper_Ab_Left));
			break;
		case AreaFlag::Mid_Ab_Left:
			regions.push_back(translator.ToRegionString(AreaFlag::Mid_Ab_Left));
			break;
		case AreaFlag::Lower_Ab_Left:
			regions.push_back(translator.ToRegionString(AreaFlag::Lower_Ab_Left));
			break;
		case AreaFlag::Forearm_Right:
			regions.push_back(translator.ToRegionString(AreaFlag::Forearm_Right));
			break;
		case AreaFlag::Upper_Arm_Right:
			regions.push_back(translator.ToRegionString(AreaFlag::Upper_Arm_Right));
			break;
		case AreaFlag::Shoulder_Right:
			regions.push_back(translator.ToRegionString(AreaFlag::Shoulder_Right));
			break;
		case AreaFlag::Back_Right:
			regions.push_back(translator.ToRegionString(AreaFlag::Back_Right));
			break;
		case AreaFlag::Chest_Right:
			regions.push_back(translator.ToRegionString(AreaFlag::Chest_Right));
			break;
		case AreaFlag::Upper_Ab_Right:
			regions.push_back(translator.ToRegionString(AreaFlag::Upper_Ab_Right));
			break;
		case AreaFlag::Mid_Ab_Right:
			regions.push_back(translator.ToRegionString(AreaFlag::Mid_Ab_Right));
			break;
		case AreaFlag::Lower_Ab_Right:
			regions.push_back(translator.ToRegionString(AreaFlag::Lower_Ab_Right));
			break;
		default:
			break;
	}

	return regions;
}

