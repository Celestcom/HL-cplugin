#pragma once
#include <boost\uuid\uuid.hpp>
#include <boost\uuid\random_generator.hpp>
#include <boost\optional.hpp>
#include "IPlayable.h"
#include "BasicHapticEvent.h"
#include <mutex>
#include "EventRegistry.h"

typedef uint32_t HapticHandle;

//todo: evaluate if this needs a name change.
//ideas: EventPlayer
//RetainedEventPlayer
//EffectPlayer

class HapticsPlayer
{
public:

	HapticsPlayer(EventRegistry& registry);
	~HapticsPlayer();

	void Update(float dt);

	void Create(HapticHandle h, std::vector<std::unique_ptr<PlayableEvent>>&& events);
	void Release(HapticHandle h);

	void Play(HapticHandle h);
	void Pause(HapticHandle h);
	void Stop(HapticHandle h);
	
	void PlayAll();
	void PauseAll();
	void ClearAll();

	boost::optional<PlayableInfo> GetHandleInfo(HapticHandle h);

	std::size_t GetNumLiveEffects();
	std::size_t GetNumReleasedEffects();
private:
	bool m_playerPaused;
	EventRegistry& m_registry;

	boost::hash<boost::uuids::uuid> m_hasher;
	boost::uuids::random_generator m_uuidGenerator;

	std::mutex m_effectsLock;

	std::unordered_map<std::size_t, std::unique_ptr<IPlayable>> m_effects;
	std::vector<std::size_t> m_frozenEffects;

	std::unordered_map<HapticHandle, boost::uuids::uuid> m_outsideToInternal;


	boost::optional<boost::uuids::uuid> findInternalHandle(HapticHandle h); 
	boost::optional<IPlayable&> findExistingPlayable(const boost::uuids::uuid& internalHandle);
	boost::optional<IPlayable&> findExistingPlayable(HapticHandle h);
	void addNewEffect(const boost::uuids::uuid&, std::vector<std::unique_ptr<PlayableEvent>>&& events);
};

