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
	m_uuidHasher(),
	m_generateUuid(),
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


int EffectPlayer::Play(EffectHandle handle)
{
	return handleCommand_synchronized(handle, [](PlayableEffect* effect) { effect->Play(); });
}

int EffectPlayer::Pause(EffectHandle handle)
{
	return handleCommand_synchronized(handle, [](PlayableEffect* effect) { effect->Pause(); });
}

int EffectPlayer::Stop(EffectHandle handle)
{
	return handleCommand_synchronized(handle, [](PlayableEffect* effect) { effect->Stop(); });
}


void EffectPlayer::Release(EffectHandle handle)
{
	handleCommand_synchronized(handle, [&](PlayableEffect* effect) {
		effect->Release();
		eraseHandle_unsynchronized(handle);
	});
}

void EffectPlayer::eraseHandle_unsynchronized(EffectHandle handle)
{
	m_outsideToInternal.erase(m_outsideToInternal.find(handle));
}



int EffectPlayer::handleCommand_synchronized(EffectHandle handle, std::function<void(PlayableEffect*)> fn)
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	if (auto effect = find_unsynchronized(handle)) {
		fn(effect);
		return HLVR_Ok;
	}
	return HLVR_Error_NoSuchHandle;
}

EffectHandle EffectPlayer::Create(std::vector<std::unique_ptr<PlayableEvent>> events)
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	
	EffectId id = nextEffectId_unsynchronized();

	PlayableEffect effect(std::move(events), m_generateUuid, m_messenger);

	m_effects.insert(std::make_pair(id.internal_hashed_uuid, std::move(effect)));

	return id.external_handle;
}

EffectPlayer::EffectId EffectPlayer::nextEffectId_unsynchronized()
{
	//If someone makes more than 4 billion haptic effects, I will be very happy!
	m_currentHandleId++;

	auto uuid = m_generateUuid();
	auto hashed_uuid = m_uuidHasher(uuid);
	m_outsideToInternal.insert(std::make_pair(m_currentHandleId, uuid));

	return EffectId{ m_currentHandleId, uuid, hashed_uuid };
}





boost::optional<EffectInfo> EffectPlayer::GetInfo(EffectHandle h) const
{
	std::lock_guard<std::mutex> guard(m_effectsLock);

	if (auto effect = find_unsynchronized(h)) {
		return effect->GetInfo();
	}
	
	return boost::none;
}


std::size_t EffectPlayer::GetNumLiveEffects() const
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	return m_effects.size() - getNumReleased_unsynchronized();
}

std::size_t EffectPlayer::GetNumReleasedEffects() const
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	return getNumReleased_unsynchronized();
}

std::size_t EffectPlayer::getNumReleased_unsynchronized() const{
	return std::accumulate(m_effects.begin(), m_effects.end(), 0, [](int total, const auto& effect) {
		return effect.second.IsReleased() ? total + 1 : total;
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





const PlayableEffect* EffectPlayer::find_unsynchronized(EffectHandle h) const 
{
	if (auto internalHandle = toInternal_unsynchronized(h)) {
		auto intKey = m_uuidHasher(*internalHandle);
		if (m_effects.find(intKey) != m_effects.end()) {
			return &m_effects.at(intKey);
		}
	}
	return nullptr;
}

PlayableEffect* EffectPlayer::find_unsynchronized(EffectHandle h)
{
	return const_cast<PlayableEffect*>(static_cast<const EffectPlayer*>(this)->find_unsynchronized(h));
}

boost::optional<boost::uuids::uuid> EffectPlayer::toInternal_unsynchronized(EffectHandle h) const
{
	if (m_outsideToInternal.find(h) != m_outsideToInternal.end()) {
		return m_outsideToInternal.at(h);
	}

	return boost::none;
}