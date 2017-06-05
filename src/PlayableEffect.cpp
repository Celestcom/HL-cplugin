#include "stdafx.h"
#include "PlayableEffect.h"
#include "Locator.h"
#include <iostream>
#include "SuitEvent.h"
#include "PriorityModel.h"
#include "NSLoader.h"
#include <iterator>

namespace NS {
	namespace Playable {
		void Restart(IPlayable& playable) {
			playable.Stop();
			playable.Play();
		}
	}
}


struct time_sorter {
	bool operator()(const SuitEvent& lhs, const SuitEvent& rhs) {
		auto visitor = TimeOffsetVisitor();
		float t1 = boost::apply_visitor(visitor, lhs);
		float t2 = boost::apply_visitor(visitor, rhs);
		return t1 < t2;
	}
};
PlayableEffect::PlayableEffect(std::vector<SuitEvent> effects,EventRegistry& reg, boost::uuids::random_generator& uuid) :
	_effects(std::move(effects)),
	_state(PlaybackState::IDLE),
	m_registry(reg),
	_id(uuid())
{
	assert(!_effects.empty());

	std::sort(_effects.begin(), _effects.end(), time_sorter());

	reset();
}



PlayableEffect::~PlayableEffect()
{
	//shouldn't have to do this anymore if you call Stop first
	//if necessary, we go through activeConsumers and stop everything
}

void PlayableEffect::Play()
{
	switch (_state) {
	case PlaybackState::IDLE:
	//	reset();
		_state = PlaybackState::PLAYING;
		break;
	case PlaybackState::PAUSED:
		resume();
		_state = PlaybackState::PLAYING;
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

	switch (_state) {
		case PlaybackState::IDLE:
			//remain in idle state
			break;
		case PlaybackState::PAUSED:
			reset();
			_state = PlaybackState::IDLE;
			break;
		case PlaybackState::PLAYING:
			reset();
			_state = PlaybackState::IDLE;
			break;
		default:
			break;
	}
}

void PlayableEffect::Pause()
{
	switch (_state) {
	case PlaybackState::IDLE:
		//remain in idle state
		break;
	case PlaybackState::PAUSED:
		//remain in paused state
		break;
	case PlaybackState::PLAYING:
		pause();
		_state = PlaybackState::PAUSED;
		break;
	default:
		break;
	}
	
	
}


void PlayableEffect::Update(float dt)
{
	if (_state == PlaybackState::IDLE || _state == PlaybackState::PAUSED) {
		return;
	}

	_time += dt;
	
	auto current(_lastExecutedEffect);
	//this visitor returns true if the event is expired and should be executed
	EventVisitor isTimeExpired(_time);

	

	
	while (current != _effects.end()) {
		if (boost::apply_visitor(isTimeExpired, *current)) {
			//region = *current->region;
			std::vector<std::string> regions = boost::apply_visitor(RegionVisitor(), *current);
			for (const auto& region : regions) {
				auto consumers = m_registry.GetEventDrivers(region); //placeholder
				if (consumers) {
					//need translator from registry's leaves to area flags for backwards compat?
					std::for_each(consumers->begin(), consumers->end(), [&](auto& consumer) {

						consumer->createRetained(_id, *current);
						m_activeDrivers.insert(std::weak_ptr<HardwareDriver>(consumer));
					});
				}
			}
			std::advance(current, 1);
		}
		else {
			
			break;
		}
	}

	_lastExecutedEffect = current;

	if (_time >= GetTotalPlayTime()) {
		Stop();
	}

} 



float PlayableEffect::GetTotalPlayTime() const
{
	TotalPlaytimeVisitor playtimeCounter;
	std::for_each(_effects.begin(), _effects.end(), boost::apply_visitor(playtimeCounter));
	return playtimeCounter.TotalPlaytime();

}

float PlayableEffect::CurrentTime() const
{
	return _time;
}

bool PlayableEffect::IsPlaying() const
{
	return _state == PlaybackState::PLAYING;
}

PlayableInfo PlayableEffect::GetInfo() const
{
	return PlayableInfo(GetTotalPlayTime(), _time, _state == PlaybackState::PLAYING);
}

void PlayableEffect::Release()
{
	std::for_each(m_activeDrivers.begin(), m_activeDrivers.end(), [&](std::weak_ptr<HardwareDriver> hd) {
		try {
			auto p = hd.lock();
			p->controlRetained(_id, NSVR_PlaybackCommand::NSVR_PlaybackCommand_Reset);
		}
		catch (std::bad_weak_ptr) {
			//the driver was removed, probably physically, so we don't need to worry about it
		}
	});
}

void PlayableEffect::reset()
{
	_time = 0;
	_lastExecutedEffect = _effects.begin();
	

	std::for_each(m_activeDrivers.begin(), m_activeDrivers.end(), [&](std::weak_ptr<HardwareDriver> hd) {
		try {
			auto p = hd.lock();
			p->controlRetained(_id, NSVR_PlaybackCommand::NSVR_PlaybackCommand_Reset);
		}
		catch (std::bad_weak_ptr) {
			//the driver was removed, probably physically, so we don't need to worry about it
		}
	});
}

void PlayableEffect::pause()
{
	std::for_each(m_activeDrivers.begin(), m_activeDrivers.end(), [&](std::weak_ptr<HardwareDriver> hd) {
		try {
			auto p = hd.lock();
			p->controlRetained(_id, NSVR_PlaybackCommand::NSVR_PlaybackCommand_Pause);
		}
		catch (std::bad_weak_ptr) {
			//the driver was removed, probably physically, so we don't need to worry about it
		}
	});
}

void PlayableEffect::resume() {
	//Now here's an interesting case. What if they unplug and switch suits with an effect paused? We want to resume on the new hardware right?
	//So if we fail to obtain the ptr, we should probably re-try to put this on a new set of drivers! But not now.

	std::for_each(m_activeDrivers.begin(), m_activeDrivers.end(), [&](std::weak_ptr<HardwareDriver> hd) {
		try {
			auto p = hd.lock();
			p->controlRetained(_id, NSVR_PlaybackCommand::NSVR_PlaybackCommand_Play);
		}
		catch (std::bad_weak_ptr) {
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

std::vector<std::string> RegionVisitor::operator()(const BasicHapticEvent & event) const
{
	auto translator = Locator::getTranslator();
	std::vector<std::string> regions;
	START_BITMASK_SWITCH(event.Area) {
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
