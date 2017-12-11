#include "stdafx.h"
#include "EffectContainer.h"
#include <experimental/unordered_map>
#include <numeric>
EffectContainer::EffectContainer()
	: m_currentHandle{0}
	, m_effects{}
	, m_frozenEffects{}
{
}

EffectContainer::EffectHandle EffectContainer::CreateEffect(PlayableEffect effect)
{
	m_currentHandle++;
	m_effects.insert(std::make_pair(m_currentHandle, std::move(effect)));
	return m_currentHandle;
}

void EffectContainer::garbageCollect()
{
	std::experimental::erase_if(m_effects, [](const auto& effect) {
		return effect.second.IsReleased() && !effect.second.IsPlaying();
	});
}

void EffectContainer::Clear()
{
	for (auto& effect : m_effects) {
		effect.second.Stop();
	}
	m_effects.clear();
}

void EffectContainer::FreezeEffects()
{
	for (auto& effect : m_effects) {
		if (effect.second.IsPlaying()) {
			effect.second.Pause();
			m_frozenEffects.push_back(effect.first);
		}
	}
}

void EffectContainer::ThawEffects()
{
	for (const auto& frozen : m_frozenEffects) {
		if (m_effects.find(frozen) != m_effects.end()) {
			m_effects.at(frozen).Play();
		}
	}

	m_frozenEffects.clear();

}

void EffectContainer::Update(float dt)
{
	for (auto& effect : m_effects) {
		effect.second.Update(dt);
	}
	
	//For simplicity, we remove old effects every update. If this is ever an issue we can make it smarter.
	garbageCollect();
}

bool EffectContainer::Mutate(EffectHandle handle, std::function<void(PlayableEffect&)> mutator)
{
	if (PlayableEffect* ptr = find(handle)) {
		if (!ptr->IsReleased()) {
			mutator(*ptr);
			return true;
		}
	}

	return false;
}

const PlayableEffect * EffectContainer::Get(EffectHandle handle) const
{
	return find(handle);
}

std::size_t EffectContainer::GetNumReleased() const
{
	return std::accumulate(m_effects.begin(), m_effects.end(), 0, [](int total, const auto& effect) {
		return effect.second.IsReleased() ? total + 1 : total;
	});
}

std::size_t EffectContainer::GetNumLive() const
{
	return m_effects.size() - GetNumReleased();
}

const PlayableEffect * EffectContainer::find(EffectHandle handle) const
{
	if (m_effects.find(handle) != m_effects.end()) {
		return &m_effects.at(handle);
	}

	return nullptr;
}

PlayableEffect * EffectContainer::find(EffectHandle handle)
{
	return const_cast<PlayableEffect*>(static_cast<const EffectContainer*>(this)->find(handle));

}
