#include "StdAfx.h"

#include "EffectPlayer.h"
#include "PlayableEffect.h"
#include "ClientMessenger.h"
#include "PlayableEffect.h"

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>

#include <experimental/vector>
#include <experimental/unordered_map>
#include <numeric>
#include <functional>
#include <chrono>





EffectPlayer::EffectPlayer(boost::asio::io_service& io, ClientMessenger& messenger)
	: m_messenger(messenger)
	, m_container()
	, m_updateHapticsInterval(boost::posix_time::millisec(5))
	, m_updateHaptics(io)
	, m_playerPaused(false)
	, m_generateUuid()
	, m_effectsLock()
{	
}

void EffectPlayer::start() 
{
	scheduleTimestep();
}


void EffectPlayer::stop()
{
	m_updateHaptics.cancel();


}

void EffectPlayer::scheduleTimestep() {
	m_updateHaptics.expires_from_now(m_updateHapticsInterval);
	m_updateHaptics.async_wait([&](auto ec) { 
		if (ec) { return; } 
		Update(m_updateHapticsInterval.total_milliseconds() / 1000.f );
		scheduleTimestep();
	});
}


void EffectPlayer::Update(float dt)
{
	std::lock_guard<std::mutex> lock_guard(m_effectsLock);

	if (m_playerPaused) {
		return;
	}

	m_container.Update(dt);
}


int EffectPlayer::Play(EffectHandle handle)
{
	return synchronized_effect_action(handle, [](PlayableEffect& effect) { effect.Play(); });
}

int EffectPlayer::Pause(EffectHandle handle)
{
	return synchronized_effect_action(handle, [](PlayableEffect& effect) { effect.Pause(); });
}

int EffectPlayer::Stop(EffectHandle handle)
{
	return synchronized_effect_action(handle, [](PlayableEffect& effect) { effect.Stop(); });
}

void EffectPlayer::Release(EffectHandle handle)
{
	synchronized_effect_action(handle, [](PlayableEffect& effect) { effect.Release(); });
}

HLVR_Result EffectPlayer::synchronized_effect_action(EffectHandle handle, std::function<void(PlayableEffect&)> fn)
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	if (m_container.Mutate(handle, fn)) {
		return HLVR_Ok;
	}
	return HLVR_Error_NoSuchHandle;
}

EffectHandle EffectPlayer::Create(std::vector<std::unique_ptr<PlayableEvent>> events)
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	
	PlayableEffect effect(std::move(events), m_generateUuid(), m_messenger);

	return m_container.CreateEffect(std::move(effect));
}


bool test() {
	return bool(boost::optional<int>{2});
}
boost::optional<EffectInfo> EffectPlayer::GetInfo(EffectHandle h) const
{
	std::lock_guard<std::mutex> guard(m_effectsLock);

	if (const PlayableEffect* effect = m_container.Get(h)) {
		return effect->GetInfo();
	}
	


	return boost::none;
}


std::size_t EffectPlayer::GetNumLiveEffects() const
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	return m_container.GetNumLive();
}

std::size_t EffectPlayer::GetNumReleasedEffects() const
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	return m_container.GetNumReleased();
}



void EffectPlayer::PlayAll()
{
	std::lock_guard<std::mutex> guard(m_effectsLock);

	m_playerPaused = false;
	
	m_container.ThawEffects();
}


void EffectPlayer::PauseAll()
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	
	m_playerPaused = true;

	m_container.FreezeEffects();
}

void EffectPlayer::ClearAll()
{
	std::lock_guard<std::mutex> guard(m_effectsLock);

	m_container.Clear();
}




