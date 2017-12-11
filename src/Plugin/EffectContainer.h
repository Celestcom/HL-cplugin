#pragma once

#include "PlayableEffect.h"

#include <unordered_map>

//This class is not thread safe; synchronization must happen at a higher level
class EffectContainer {
public:
	EffectContainer();
	using EffectHandle = uint32_t;
	EffectHandle CreateEffect(PlayableEffect effect);

	void Clear();
	void FreezeEffects();
	void ThawEffects();
	
	void Update(float dt);

	bool Mutate(EffectHandle handle, std::function<void(PlayableEffect&)>);
	const PlayableEffect* Get(EffectHandle handle) const;

	std::size_t GetNumReleased() const;
	std::size_t GetNumLive() const;
private:
	EffectHandle m_currentHandle;
	std::unordered_map<EffectHandle, PlayableEffect> m_effects;
	std::vector<EffectHandle> m_frozenEffects;

	const PlayableEffect* find(EffectHandle handle) const;
	PlayableEffect* find(EffectHandle handle);
	void garbageCollect();

};


