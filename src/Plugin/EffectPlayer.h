#pragma once

#include "PlayableEffect.h"

#include <boost/uuid/uuid.hpp> 
#include <boost/uuid/random_generator.hpp> 
#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <mutex>
#include <atomic>
#include <unordered_map>




using EffectHandle = uint32_t;


class ClientMessenger;
class EventRegistry;
class EffectPlayer
{
public:
	//We need io_service for our async update loop, and ClientMessenger to write to shared memory
	EffectPlayer(boost::asio::io_service& io, ClientMessenger& messenger);

	EffectHandle Create(std::vector<std::unique_ptr<PlayableEvent>> events);
	void Release(EffectHandle handle);

	int Play(EffectHandle handle);
	int Pause(EffectHandle handle);
	int Stop(EffectHandle handle);
	
	void PlayAll();
	void PauseAll();
	void ClearAll();

	void Update(float dt);

	boost::optional<EffectInfo> GetInfo(EffectHandle h) const;

	std::size_t GetNumLiveEffects() const;
	std::size_t GetNumReleasedEffects() const;

	void start();
	void stop();
private:
	struct EffectId {
		EffectHandle external_handle;
		boost::uuids::uuid internal_uuid;
		std::size_t internal_hashed_uuid;
	};
	ClientMessenger& m_messenger;

	boost::posix_time::millisec m_updateHapticsInterval;
	boost::asio::deadline_timer m_updateHaptics;

	bool m_playerPaused;

	boost::hash<boost::uuids::uuid> m_uuidHasher;
	boost::uuids::random_generator m_generateUuid;

	mutable std::mutex m_effectsLock;

	std::unordered_map<std::size_t, PlayableEffect> m_effects;
	std::vector<std::size_t> m_frozenEffects;

	std::unordered_map<EffectHandle, boost::uuids::uuid> m_outsideToInternal;

	uint32_t m_currentHandleId;
	
	
	void scheduleTimestep();

	boost::optional<boost::uuids::uuid> toInternal_unsynchronized(EffectHandle h) const; 
	const PlayableEffect* find_unsynchronized(EffectHandle h) const;
	PlayableEffect* find_unsynchronized(EffectHandle h);
	
	EffectId nextEffectId_unsynchronized();

	int handleCommand_synchronized(EffectHandle handle, std::function<void(PlayableEffect*)>);

	std::size_t getNumReleased_unsynchronized() const;

	void eraseHandle_unsynchronized(EffectHandle handle);


};

