#pragma once
#include "IPlayable.h"
#include "HapticEvent.h"
#include "HapticEventGenerator.h"
#include "SuitEvent.h"
#include "EventRegistry.h"
#include <set>
#include <boost\uuid\random_generator.hpp>

namespace NS {
	namespace Playable {
		//implements Restart by calling Stop() followed by Play()
		void Restart(const std::unique_ptr<IPlayable>&);
	}
}

//Responsible for calling the relevant components to execute each type of event
class EventExecutor : public boost::static_visitor<> {
private:
	//Main ID of the effect
	boost::uuids::uuid& m_id;
	EventRegistry& m_registry;
public:
	EventExecutor(boost::uuids::uuid& id, EventRegistry& registry);

	void operator()(BasicHapticEvent& hapticEvent);
};

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


class PlayableEffect :
	public IPlayable
{
public:
	
	//Precondition: the vector is not empty
	PlayableEffect(std::vector<SuitEvent> effects, EventRegistry& reg, boost::uuids::random_generator&);
	~PlayableEffect();

	void Play() override;
	void Pause() override;
	void Stop() override;
	void Update(float dt) override;
	float GetTotalPlayTime() const override;
	float CurrentTime() const override;
	bool IsPlaying() const override;
	PlayableInfo GetInfo() const override;
	void Release() override;

	
private:
	enum class PlaybackState {
		PLAYING,
		PAUSED,
		IDLE
	};
	
	PlaybackState _state;

	float _time;
	EventRegistry& m_registry;
	
	std::set<std::weak_ptr<HardwareDriver>> m_activeDrivers;
	std::vector<SuitEvent>::iterator _lastExecutedEffect;
	std::vector<SuitEvent> _effects;
	boost::uuids::uuid _id;

	void reset();
	void pause();
	void resume();

};

