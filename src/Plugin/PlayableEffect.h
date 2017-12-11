#pragma once
#include "PlayableEvent.h"

#include <boost/uuid/uuid.hpp>
#include <vector>
#include <memory>


//The purpose of this class is to hold a bunch of events together in a timeline - an Effect. 
//This is the actual object that a user of the Hardlight SDK is interacting with when they make haptic effects.
//It contains facilities for controlling playback of an effect, as well as releasing it when they are done. 

//Used to group together some common info about an effect, for use at higher levels of the SDK
struct EffectInfo {
	float Duration;
	float CurrentTime;
	int State;
};


using PlayablePtr = std::unique_ptr<PlayableEvent>;

void sortByTime(std::vector<PlayablePtr>* playables);
void removeDuplicates(std::vector<PlayablePtr>* playables);


class ClientMessenger;

class PlayableEffect 
{
public:

	//Precondition: effects.size() > 0
	PlayableEffect(std::vector<PlayablePtr> effects, boost::uuids::uuid id, ClientMessenger& messenger);

	//Can't be copied - that would break the internal effect iterator 
	PlayableEffect(const PlayableEffect&) = delete;

	//But can be moved - will not break internal effect iterator
	PlayableEffect(PlayableEffect&&) = default;

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



	void scrubToBegin();
	void reset();
	void pause();
	void resume();

};

