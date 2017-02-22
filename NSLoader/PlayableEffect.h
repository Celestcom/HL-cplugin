#pragma once
#include "IPlayable.h"
#include "Wire\IntermediateHapticFormats.h"
#include "HapticEvent.h"
#include "HapticEventGenerator.h"
#include "Wire\FlatbuffDecoder.h"
namespace NS {
	namespace Playable {
		void Restart(const std::unique_ptr<IPlayable>&);
	}
}

class BasicHapticEventVisitor : public boost::static_visitor<BasicHapticEvent> {
public:
	bool operator()(BasicHapticEvent& h, HapticEventGenerator& gen, boost::uuids::uuid id) {
		if (h.Time)
		gen.NewEvent(AreaFlag(h.Area), h.Duration, h.Effect, h.Strength, id);
	}
};

class EventVisitor : public boost::static_visitor<> {
private:
	float m_time;
public: 

	EventVisitor(float time);
	template <typename T>
	void operator(T& operand) const {
		return operand.Time <= m_time;
	}
};
class PlayableEffect :
	public IPlayable
{
public:
	
	//Precondition: the vector is not empty
	PlayableEffect(std::vector<FlatbuffDecoder::SuitEvent> effects, HapticEventGenerator& gen);
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
	HapticEventGenerator& _gen;

	std::vector<FlatbuffDecoder::SuitEvent>::iterator _lastExecutedEffect;
	std::vector<FlatbuffDecoder::SuitEvent> _effects;
	boost::uuids::uuid _id;

	void reset();
	void pause();
	void resume();

};

