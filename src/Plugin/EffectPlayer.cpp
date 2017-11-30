#include "StdAfx.h"

#include "EffectPlayer.h"
#include "PlayableEffect.h"
#include "ClientMessenger.h"

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>

#include <experimental/vector>
#include <experimental/unordered_map>
#include <numeric>
#include <functional>
#include <chrono>





EffectPlayer::EffectPlayer(boost::asio::io_service& io, ClientMessenger& messenger):
	m_messenger(messenger),
	m_updateHapticsInterval(boost::posix_time::millisec(5)),
	m_updateHaptics(io),
	m_playerPaused(false),
	m_hasher(),
	m_uuidGenerator(),
	m_effectsLock(),
	m_effects(),
	m_frozenEffects(),
	m_outsideToInternal(),
	m_currentHandleId(0)

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

int EffectPlayer::synchronizedHandleCommand(EffectHandle handle, std::function<void(PlayableEffect*)> fn)
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	if (auto effect = find(handle)) {
		fn(effect);
		return HLVR_Ok;
	}
	return HLVR_Error_NoSuchHandle;
}

int EffectPlayer::Play(EffectHandle handle)
{
	return synchronizedHandleCommand(handle, [](PlayableEffect* effect) { effect->Play(); });
}

int EffectPlayer::Pause(EffectHandle handle)
{
	return synchronizedHandleCommand(handle, [](PlayableEffect* effect) { effect->Pause(); });
}

int EffectPlayer::Stop(EffectHandle handle)
{
	return synchronizedHandleCommand(handle, [](PlayableEffect* effect) { effect->Stop(); });
}



void EffectPlayer::Release(EffectHandle handle)
{
	synchronizedHandleCommand(handle, [&](PlayableEffect* effect) {
		effect->Release();
		m_outsideToInternal.erase(m_outsideToInternal.find(handle));
	});
}

EffectHandle EffectPlayer::nextHandle()
{
	//This will overflow when we exceed the storage of a uint32_t (4 billion). 
	//Update if this becomes likely: someone playing constant effects 10 times per second on 16 pads for ~300 days.
	m_currentHandleId++;
	return EffectHandle(m_currentHandleId);
}


EffectHandle EffectPlayer::Create(std::vector<std::unique_ptr<PlayableEvent>> events)
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	
	EffectHandle handle = nextHandle();
	boost::uuids::uuid uuid = m_uuidGenerator();
	m_outsideToInternal.insert(std::make_pair(handle, uuid));
	addNewEffect(uuid, std::move(events));

	return handle;
}



void EffectPlayer::addNewEffect(const boost::uuids::uuid& id, std::vector<std::unique_ptr<PlayableEvent>>&& events) {
	
	PlayableEffect temp(std::move(events),  m_uuidGenerator, m_messenger);
	m_effects.insert(std::make_pair(m_hasher(id), std::move(temp)));
}


boost::optional<EffectInfo> EffectPlayer::GetInfo(EffectHandle h) const
{
	std::lock_guard<std::mutex> guard(m_effectsLock);

	if (auto effect = find(h)) {
		return effect->GetInfo();
	}
	
	return boost::none;
}


std::size_t EffectPlayer::GetNumLiveEffects() const
{
	m_effectsLock.lock();
	std::size_t totalEffects = m_effects.size();
	m_effectsLock.unlock();

	return totalEffects - GetNumReleasedEffects();
}

std::size_t EffectPlayer::GetNumReleasedEffects() const
{
	std::lock_guard<std::mutex> guard(m_effectsLock);

	return std::accumulate(m_effects.begin(), m_effects.end(), 0, [](int currentTotal, const auto& effect) {
		return effect.second.IsReleased() ? currentTotal + 1 : currentTotal;
	});
}


void EffectPlayer::Update(float dt)
{

	std::lock_guard<std::mutex> lock_guard(m_effectsLock);

	if (m_playerPaused) {
		return;
	}

	for (auto& effect : m_effects) {
		effect.second.Update(dt);
	}

	std::experimental::erase_if(m_effects, [](const auto& effect) {
		return effect.second.IsReleased() && !effect.second.IsPlaying();
	});

}


void EffectPlayer::PlayAll()
{
	std::lock_guard<std::mutex> guard(m_effectsLock);

	m_playerPaused = false;

	
	for (const auto& frozen : m_frozenEffects) {
		if (m_effects.find(frozen) != m_effects.end()) {
			m_effects.at(frozen).Play();
		}
	}

	m_frozenEffects.clear();
}


void EffectPlayer::PauseAll()
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	
	m_playerPaused = true;

	for (auto& effect : m_effects) {
		if (effect.second.IsPlaying()) {
			effect.second.Pause();
			m_frozenEffects.push_back(effect.first);
		}
	}
}

void EffectPlayer::ClearAll()
{
	std::lock_guard<std::mutex> guard(m_effectsLock);

	for (auto& effect : m_effects) {
		effect.second.Stop();
	}

	m_outsideToInternal.clear();
	m_effects.clear();
}


boost::optional<boost::uuids::uuid> EffectPlayer::toInternal(EffectHandle h) const
{
	if (m_outsideToInternal.find(h) != m_outsideToInternal.end()) {
		return m_outsideToInternal.at(h);
	}

	return boost::none;
}


const PlayableEffect* EffectPlayer::find(EffectHandle h) const 
{
	if (auto internalHandle = toInternal(h)) {
		auto intKey = m_hasher(*internalHandle);
		if (m_effects.find(intKey) != m_effects.end()) {
			return &m_effects.at(intKey);
		}
	}
	return nullptr;
}

PlayableEffect* EffectPlayer::find(EffectHandle h)
{
	return const_cast<PlayableEffect*>(static_cast<const EffectPlayer*>(this)->find(h));
}
