#pragma once

#include "EffectContainer.h"
#include <boost/uuid/random_generator.hpp> 
#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <mutex>





using EffectHandle = uint32_t;

class PlayableEffect;
class ClientMessenger;
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
	EffectContainer m_container;

	boost::posix_time::millisec m_updateHapticsInterval;
	boost::asio::deadline_timer m_updateHaptics;
	void scheduleTimestep();


	bool m_playerPaused;

	boost::uuids::random_generator m_generateUuid;

	mutable std::mutex m_effectsLock;
	HLVR_Result synchronized_effect_action(EffectHandle handle, std::function<void(PlayableEffect&)> fn);



	


};

