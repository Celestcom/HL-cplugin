#include "StdAfx.h"
#include "HapticsPlayer.h"
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>
#include <experimental/vector>
#include <experimental/unordered_map>
#include <numeric>
#include "PlayableEffect.h"
using namespace std;


HapticsPlayer::HapticsPlayer(EventRegistry& registry):
	m_playerPaused(false),
	m_effectsLock(),
	m_registry(registry)
{	
}

HapticsPlayer::~HapticsPlayer()
{
}

void HapticsPlayer::Play(HapticHandle hh)
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	if (auto effect = findExistingPlayable(hh)) {
		effect->Play();
	}
}

void HapticsPlayer::Pause(HapticHandle hh)
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	if (auto effect = findExistingPlayable(hh)) {
		effect->Pause();
	}
}



void HapticsPlayer::Stop(HapticHandle hh)
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	if (auto effect = findExistingPlayable(hh)) {
		effect->Stop();
	}
}

void HapticsPlayer::Release(HapticHandle h)
{
	std::lock_guard<std::mutex> guard(m_effectsLock);
	
	if (auto effect = findExistingPlayable(h)) {
		effect->Release();
		m_outsideToInternal.erase(m_outsideToInternal.find(h));
	}
}

void HapticsPlayer::Create(HapticHandle h, std::vector<std::unique_ptr<PlayableEvent>>&& events)
{
	std::lock_guard<std::mutex> guard(m_effectsLock);

	if (auto internalHandle = findInternalHandle(h)) {
		if (auto effect = findExistingPlayable(*internalHandle)) {
			effect->Stop();
			addNewEffect(*internalHandle, std::move(events));
		} else {
			assert("It should never be the case that an internal handle is found, \
				yet the playable it is associated with is not");
		}
	}
	else {
		boost::uuids::uuid uuid = m_uuidGenerator();
		m_outsideToInternal[h] = uuid;
		addNewEffect(uuid, std::move(events));
	}
}



void HapticsPlayer::addNewEffect(const boost::uuids::uuid& id, std::vector<std::unique_ptr<PlayableEvent>>&& events) {
	m_effects[m_hasher(id)] = std::unique_ptr<IPlayable>(
		new PlayableEffect(std::move(events), m_registry, m_uuidGenerator)
	);
}

boost::optional<PlayableInfo> HapticsPlayer::GetHandleInfo(HapticHandle h) 
{
	std::lock_guard<std::mutex> guard(m_effectsLock);

	if (auto effect = findExistingPlayable(h)) {
		return effect->GetInfo();
	}
	
	return boost::none;
}


std::size_t HapticsPlayer::NumLiveEffects()
{
	return m_effects.size();
}

std::size_t HapticsPlayer::NumOrphanedEffects()
{
	return std::accumulate(m_effects.begin(), m_effects.end(), 0, [](int currentTotal, const auto& effect) {
		return effect.second->IsReleased() ? currentTotal + 1 : currentTotal;
	});
}


void HapticsPlayer::Update(float dt)
{
	std::lock_guard<std::mutex> lock_guard(m_effectsLock);

	
	for (auto& effect : m_effects) {
		effect.second->Update(dt);
	}

	std::experimental::erase_if(m_effects, [](const auto& effect) { 
		return effect.second->IsReleased() && !effect.second->IsPlaying(); 
	});


}


void HapticsPlayer::PlayAll()
{
	std::lock_guard<std::mutex> guard(m_effectsLock);

	m_playerPaused = false;

	
	for (const auto& frozen : m_frozenEffects) {
		if (m_effects.find(frozen) != m_effects.end()) {
			m_effects.at(frozen)->Play();
		}
	}

	m_frozenEffects.clear();
}


void HapticsPlayer::PauseAll()
{
	m_playerPaused = true;

	std::lock_guard<std::mutex> guard(m_effectsLock);

	for (auto& effect : m_effects) {
		if (effect.second->IsPlaying()) {
			effect.second->Pause();
			m_frozenEffects.push_back(effect.first);
		}
	}
}

void HapticsPlayer::ClearAll()
{
	std::lock_guard<std::mutex> guard(m_effectsLock);

	for (auto& effect : m_effects) {
		effect.second->Stop();
	}
	m_outsideToInternal.clear();
	m_effects.clear();
}





boost::optional<boost::uuids::uuid> HapticsPlayer::findInternalHandle(HapticHandle h)
{
	if (m_outsideToInternal.find(h) != m_outsideToInternal.end()) {
		return m_outsideToInternal.at(h);
	}

	return boost::none;
}

boost::optional<IPlayable&> HapticsPlayer::findExistingPlayable(const boost::uuids::uuid& internalHandle)
{
	auto intKey = m_hasher(internalHandle);
	if (m_effects.find(intKey) != m_effects.end()) {
		return *m_effects.at(intKey);
	}

	return boost::none;
}

boost::optional<IPlayable&> HapticsPlayer::findExistingPlayable(HapticHandle h)
{
	if (auto internalHandle = findInternalHandle(h)) {
		return findExistingPlayable(*internalHandle);
	}

	return boost::none;
}

