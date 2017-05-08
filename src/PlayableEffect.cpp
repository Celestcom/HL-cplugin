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
		void Restart(const std::unique_ptr<IPlayable>& playable) {
			playable->Stop();
			playable->Play();
		}
	}
}

PlayableEffect::PlayableEffect(std::vector<SuitEvent> effects,EventRegistry& reg, boost::uuids::random_generator& uuid) :
	_effects(std::move(effects)),
	_state(PlaybackState::IDLE),
	m_registry(reg),
	_id(uuid())
{
	assert(!_effects.empty());
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
			auto consumers = m_registry.GetEventDrivers("body"); //placeholder
			std::for_each(consumers->begin(), consumers->end(), [&](auto& consumer) {
				
				consumer->createRetained(_id, *current);
				m_activeDrivers.insert(std::weak_ptr<HardwareDriver>(consumer));
			});
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

EventExecutor::EventExecutor(boost::uuids::uuid & id, EventRegistry& registry):
	m_id(id), 
	m_registry(registry)
{
}

void EventExecutor::operator()(BasicHapticEvent & h)
{
	auto consumers = m_registry.GetEventDrivers("body"); // todo: needs actual region
//	std::for_each(consumers->begin(), consumers->end(), [](auto& consumer) {consumer->createRetained()}
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
