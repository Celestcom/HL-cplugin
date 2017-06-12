#pragma once
#include <boost\uuid\uuid.hpp>
#include <boost\uuid\random_generator.hpp>
#include <boost\optional.hpp>
#include "IPlayable.h"
#include "BasicHapticEvent.h"
#include <mutex>
#include "EventRegistry.h"

typedef unsigned int HapticHandle;

//todo: evaluate if this needs a name change.
//ideas: EventPlayer
//RetainedEventPlayer
//EffectPlayer

class HapticsPlayer
{
public:

	struct EffectInfo {
		uint16_t strength;
		uint32_t family;
		AreaFlag area;
		EffectInfo(uint16_t strength, uint32_t family, AreaFlag area) : strength(strength), family(family), area(area) {}
	};

	struct Released {
	public:
		boost::uuids::uuid ID;
		bool NeedsSweep;
		Released(boost::uuids::uuid id) :ID(id), NeedsSweep(false) {}
	};

	
	HapticsPlayer(EventRegistry& registry);
	~HapticsPlayer();

	void Update(float dt);
	void Play(HapticHandle h);
	void Pause(HapticHandle h);
	void Restart(HapticHandle h);
	void Stop(HapticHandle h);
	void Release(HapticHandle h);
	void Create(HapticHandle h, std::vector<std::unique_ptr<PlayableEvent>>&& events);
	boost::optional<PlayableInfo> GetHandleInfo(HapticHandle h);

	std::size_t NumLiveEffects();
	std::size_t NumOrphanedEffects();

	void PlayAll();
	void PauseAll();
	void ClearAll();

	

	std::vector<EffectInfo> GetEffectInfo() const;
	
private:
	boost::hash<boost::uuids::uuid> uuid_hasher;
	boost::uuids::random_generator _uuidGen;

	std::mutex m_effectsMutex;
	std::unordered_map<std::size_t, std::unique_ptr<IPlayable>> _effects;


	std::vector<Released> _releasedEffects;
	std::unordered_map<HapticHandle, boost::uuids::uuid> _outsideHandleToUUID;
	std::vector<std::size_t> _frozenEffects;

	bool _paused;

	EventRegistry& m_registry;
	boost::optional<IPlayable&>  toInternalUUID(HapticHandle hh) const;

};

