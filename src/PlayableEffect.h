#pragma once
#include "IPlayable.h"
#include "BasicHapticEvent.h"
#include "EventRegistry.h"
#include <set>
#include <boost\uuid\random_generator.hpp>
#include "HardwareDriver.h"

template<typename T>
struct weak_ptr_less_than {
	bool operator() (const std::weak_ptr<T>& lhs, const std::weak_ptr<T>& rhs) const {
		return lhs.expired() || (!rhs.expired() && *lhs.lock() < *rhs.lock());
	}
};


std::vector<std::string> extractRegions(const std::unique_ptr<PlayableEvent> & event);




class RegionVisitor : public boost::static_visitor<std::vector<std::string>> {
private:
public:
	RegionVisitor();
	std::vector<std::string> operator()(const BasicHapticEvent& event) const;
};
using PlayablePtr = std::unique_ptr<PlayableEvent>;

class PlayableEffect 
{
public:


	//Precondition: the vector is not empty
	PlayableEffect(std::vector<PlayablePtr>&& effects, EventRegistry& reg, boost::uuids::random_generator&);
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
	PlayableInfo GetInfo() const;
	void Release();

	
private:
	void pruneDuplicates(std::vector<PlayablePtr>& playables);
	void sortByTime(std::vector<PlayablePtr>& playables);

	enum class PlaybackState {
		PLAYING,
		PAUSED,
		IDLE
	};
	
	PlaybackState m_state;

	float m_time;
	EventRegistry& m_registry;
	std::vector<std::unique_ptr<PlayableEvent>> m_effects;

	std::set<std::weak_ptr<HardwareDriver>, weak_ptr_less_than<HardwareDriver>> m_activeDrivers;
	std::vector<std::unique_ptr<PlayableEvent>>::iterator m_lastExecutedEffect;
	boost::uuids::uuid m_id;

	bool m_released;

	void scrubToBegin();
	void reset();
	void pause();
	void resume();

};

