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
	ClientMessenger& m_messenger;

	boost::posix_time::millisec m_updateHapticsInterval;
	boost::asio::deadline_timer m_updateHaptics;

	bool m_playerPaused;

	boost::hash<boost::uuids::uuid> m_hasher;
	boost::uuids::random_generator m_uuidGenerator;

	mutable std::mutex m_effectsLock;

	std::unordered_map<std::size_t, PlayableEffect> m_effects;
	std::vector<std::size_t> m_frozenEffects;

	std::unordered_map<EffectHandle, boost::uuids::uuid> m_outsideToInternal;

	std::atomic<uint32_t> m_currentHandleId;

	boost::optional<boost::uuids::uuid> toInternal(EffectHandle h) const; 
	const PlayableEffect* find(EffectHandle h) const;
	PlayableEffect* find(EffectHandle h);
	
	void addNewEffect(const boost::uuids::uuid&, std::vector<std::unique_ptr<PlayableEvent>>&& events);
	EffectHandle nextHandle();

	void scheduleTimestep();

	int synchronizedHandleCommand(EffectHandle handle, std::function<void(PlayableEffect*)>);


	

};

