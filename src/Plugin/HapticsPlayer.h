#pragma once
#include <boost\uuid\uuid.hpp>
#include <boost\uuid\random_generator.hpp>
#include <boost\functional\hash.hpp>
#include <boost\optional.hpp>
#include "PlayableEffect.h"
#include <mutex>
#include <atomic>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using HapticHandle = uint32_t;

//todo: evaluate if this needs a name change.
//ideas: EventPlayer
//RetainedEventPlayer
//EffectPlayer

class ClientMessenger;
class EventRegistry;
class HapticsPlayer
{
public:

	HapticsPlayer(boost::asio::io_service& io, ClientMessenger& messenger);


	HapticHandle Create(std::vector<std::unique_ptr<PlayableEvent>> events);
	void Release(HapticHandle h);

	void start();
	void stop();
	void Update(float dt);
	int Play(HapticHandle h);
	int Pause(HapticHandle h);
	int Stop(HapticHandle h);
	
	void PlayAll();
	void PauseAll();
	void ClearAll();

	boost::optional<PlayableInfo> GetHandleInfo(HapticHandle h) const;

	std::size_t GetNumLiveEffects();
	std::size_t GetNumReleasedEffects();
private:
	bool m_playerPaused;

	boost::hash<boost::uuids::uuid> m_hasher;
	boost::uuids::random_generator m_uuidGenerator;

	mutable std::mutex m_effectsLock;

	std::unordered_map<std::size_t, PlayableEffect> m_effects;
	std::vector<std::size_t> m_frozenEffects;

	std::unordered_map<HapticHandle, boost::uuids::uuid> m_outsideToInternal;

	std::atomic<uint32_t> m_currentHandleId;

	boost::optional<boost::uuids::uuid> findInternalHandle(HapticHandle h) const; 
	const PlayableEffect* findExistingPlayable(HapticHandle h) const;
	PlayableEffect* findExistingPlayable(HapticHandle h);
	void addNewEffect(const boost::uuids::uuid&, std::vector<std::unique_ptr<PlayableEvent>>&& events);
	HapticHandle nextHandle();

	void scheduleTimestep();

	ClientMessenger& m_messenger;

	boost::posix_time::millisec m_updateHapticsInterval;
	boost::asio::deadline_timer m_updateHaptics;

};

