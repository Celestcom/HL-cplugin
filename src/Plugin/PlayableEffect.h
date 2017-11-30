#pragma once
#include <set>
#include <boost\uuid\random_generator.hpp>
#include "PlayableEvent.h"
template<typename T>
struct weak_ptr_less_than {
	bool operator() (const std::weak_ptr<T>& lhs, const std::weak_ptr<T>& rhs) const {
		return lhs.expired() || (!rhs.expired() && *lhs.lock() < *rhs.lock());
	}
};

struct EffectInfo {
	float Duration;
	float CurrentTime;
	int State;
};
std::vector<std::string> extractRegions(const std::unique_ptr<PlayableEvent> & event);




using PlayablePtr = std::unique_ptr<PlayableEvent>;

class ClientMessenger;

class PlayableEffect 
{
public:


	//Precondition: the vector is not empty
	PlayableEffect(std::vector<PlayablePtr>&& effects, boost::uuids::random_generator& gen, ClientMessenger& messenger);
	PlayableEffect(const PlayableEffect&) = delete;
	PlayableEffect(PlayableEffect&&);
	~PlayableEffect();
	void Play();
	void Pause();
	void Stop();
	void Update(float dt);
	float GetTotalDuration() const;
	float CurrentTime() const;
	bool IsPlaying() const;
	bool IsReleased() const;
	EffectInfo GetInfo() const;
	void Release();

	
private:
	void pruneDuplicates(std::vector<PlayablePtr>& playables);
	void sortByTime(std::vector<PlayablePtr>& playables);

	enum class PlaybackState {
	
		PLAYING = 1,
		PAUSED,
		IDLE
	};
	
	PlaybackState m_state;

	float m_time;
	std::vector<std::unique_ptr<PlayableEvent>> m_effects;

	std::vector<std::unique_ptr<PlayableEvent>>::iterator m_lastExecutedEffect;
	boost::uuids::uuid m_id;

	ClientMessenger& m_messenger;
	bool m_released;

	void scrubToBegin();
	void reset();
	void pause();
	void resume();

};

