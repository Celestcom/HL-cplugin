#pragma once
#include "IPlayable.h"
#include "BasicHapticEvent.h"
#include "EventRegistry.h"
#include <set>
#include <boost\uuid\random_generator.hpp>


template<typename T>
struct weak_ptr_less_than {
	bool operator() (const std::weak_ptr<T>& lhs, const std::weak_ptr<T>& rhs) const {
		return lhs.expired() || (!rhs.expired() && *lhs.lock() < *rhs.lock());
	}
};
namespace NS {
	namespace Playable {
		//implements Restart by calling Stop() followed by Play()
		void Restart(IPlayable&);
	}
}



//Responsible for summing up the duration of an effect
class TotalPlaytimeVisitor : public boost::static_visitor<> {
private:
	float m_totalPlaytime;
	//A basic effect needs a minimum duration, which in practice is about 0.25 seconds
	const float m_fudgeFactor;
public:
	TotalPlaytimeVisitor();
	template<typename T>
	void operator()(T& operand) {
		float thisEffectEndTime = std::max(0.0f, operand.Time + operand.Duration) + m_fudgeFactor;
		m_totalPlaytime = std::max(m_totalPlaytime, thisEffectEndTime);
	}

	float TotalPlaytime();
};

std::vector<std::string> extractRegions(const std::unique_ptr<PlayableEvent> & event);

//Responsible for checking if an event has expired
class EventVisitor : public boost::static_visitor<bool> {
private:
	float m_time;
public: 
	EventVisitor(float time);
	template <typename T>
	bool operator()(T& operand) const {
		return operand.Time <= m_time;
	}
};


class RegionVisitor : public boost::static_visitor<std::vector<std::string>> {
private:
public:
	RegionVisitor();
	std::vector<std::string> operator()(const BasicHapticEvent& event) const;
};
using PlayablePtr = std::unique_ptr<PlayableEvent>;

class PlayableEffect :
	public IPlayable
{
public:

	//Precondition: the vector is not empty
	PlayableEffect(std::vector<PlayablePtr>&& effects, EventRegistry& reg, boost::uuids::random_generator&);
	~PlayableEffect();
	
	void Play() override;
	void Pause() override;
	void Stop() override;
	void Update(float dt) override;
	float GetTotalPlayTime() const override;
	float CurrentTime() const override;
	bool IsPlaying() const override;
	bool IsReleased() const override;
	PlayableInfo GetInfo() const override;
	void Release() override;

	
private:
	enum class PlaybackState {
		PLAYING,
		PAUSED,
		IDLE
	};
	
	PlaybackState m_state;

	float m_time;
	EventRegistry& m_registry;

	std::set<std::weak_ptr<HardwareDriver>, weak_ptr_less_than<HardwareDriver>> m_activeDrivers;
	std::vector<PlayablePtr>::iterator m_lastExecutedEffect;
	std::vector<PlayablePtr> m_effects;
	boost::uuids::uuid m_id;

	bool m_released;

	void reset();
	void pause();
	void resume();

};

