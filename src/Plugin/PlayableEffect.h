#pragma once
#include "PlayableEvent.h"

#include <boost/uuid/uuid.hpp>
#include <vector>
#include <memory>

//Used to group together some common info about an effect, for use at higher levels of the SDK
struct EffectInfo {
	float Duration;
	float CurrentTime;
	int State;
};


using PlayablePtr = std::unique_ptr<PlayableEvent>;

class ClientMessenger;

class PlayableEffect 
{
public:

	//Precondition: effects.size() > 0
	PlayableEffect(std::vector<PlayablePtr> effects, boost::uuids::uuid id, ClientMessenger& messenger);

	void Play();
	void Pause();
	void Stop();

	void Update(float dt);

	float GetTotalDuration() const;
	float CurrentTime() const;
	bool IsPlaying() const;
	bool IsReleased() const;
	void Release();

	EffectInfo GetInfo() const;
	
private:
	enum class PlaybackState {
		PLAYING = 1,
		PAUSED,
		IDLE
	};

	PlaybackState m_state;
	float m_time;
	std::vector<std::unique_ptr<PlayableEvent>> m_effects;
	decltype(m_effects)::iterator m_lastExecutedEffect;
	boost::uuids::uuid m_id;
	ClientMessenger& m_messenger;
	bool m_isReleased;

	void pruneDuplicates(std::vector<PlayablePtr>& playables);
	void sortByTime(std::vector<PlayablePtr>& playables);


	void scrubToBegin();
	void reset();
	void pause();
	void resume();

};

