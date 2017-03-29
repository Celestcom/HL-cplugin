#include "stdafx.h"
#include "PlayableEffect.h"
#include "Locator.h"
#include "Wire\IntermediateHapticFormats.h"
#include <iostream>
#include "PriorityModel.h"
#include <iterator>
namespace NS {
	namespace Playable {
		void Restart(const std::unique_ptr<IPlayable>& playable) {
			playable->Stop();
			playable->Play();
		}
	}
}

PlayableEffect::PlayableEffect(std::vector<FlatbuffDecoder::SuitEvent> effects, HapticEventGenerator& gen, boost::uuids::random_generator& uuid) :
	_effects(std::move(effects)),
	_state(PlaybackState::IDLE),
	_gen(gen),
	_id(uuid())
{
	assert(!_effects.empty());
	reset();
}



PlayableEffect::~PlayableEffect()
{
	try {
		_gen.Remove(_id);
	}
	catch (const std::exception& e) {
		//todo: NEED TO LOG
	}
}

void PlayableEffect::Play()
{
	switch (_state) {
	case PlaybackState::IDLE:
	//	reset();
		_state = PlaybackState::PLAYING;
		break;
	case PlaybackState::PAUSED:
		resume();
		_state = PlaybackState::PLAYING;
		break;
	case PlaybackState::PLAYING:
		//remain in playing state
		break;
	default:
		break;
	}
	
}

void PlayableEffect::Stop()
{

	switch (_state) {
		case PlaybackState::IDLE:
			//remain in idle state
			break;
		case PlaybackState::PAUSED:
			reset();
			_state = PlaybackState::IDLE;
			break;
		case PlaybackState::PLAYING:
			reset();
			_state = PlaybackState::IDLE;
			break;
		default:
			break;
	}
}

void PlayableEffect::Pause()
{
	switch (_state) {
	case PlaybackState::IDLE:
		//remain in idle state
		break;
	case PlaybackState::PAUSED:
		//remain in paused state
		break;
	case PlaybackState::PLAYING:
		pause();
		_state = PlaybackState::PAUSED;
		break;
	default:
		break;
	}
	
	
}


void PlayableEffect::Update(float dt)
{
	if (_state == PlaybackState::IDLE || _state == PlaybackState::PAUSED) {
		return;
	}

	_time += dt;
	
	auto current(_lastExecutedEffect);
	//this visitor returns true if the event is expired and should be executed
	EventVisitor isTimeExpired(_time);

	//this visitor actually executes the event
	EventExecutor executeEvent(_id, _gen);


	while (current != _effects.end()) {
		if (boost::apply_visitor(isTimeExpired, *current)) {
			boost::apply_visitor(executeEvent, *current);
			std::advance(current, 1);
		}
		else {
			
			break;
		}
//		else {
			//_lastExecutedEffect = current; //<-- this is a noop? confirm
		//	break;
	//	}
	
	}

	_lastExecutedEffect = current;

	if (_time >= GetTotalPlayTime()) {
		Stop();
	}

} 



float PlayableEffect::GetTotalPlayTime() const
{
	TotalPlaytimeVisitor playtimeCounter;
	std::for_each(_effects.begin(), _effects.end(), boost::apply_visitor(playtimeCounter));
	return playtimeCounter.TotalPlaytime();

}

float PlayableEffect::CurrentTime() const
{
	return _time;
}

bool PlayableEffect::IsPlaying() const
{
	return _state == PlaybackState::PLAYING;
}

PlayableInfo PlayableEffect::GetInfo() const
{
	return PlayableInfo(GetTotalPlayTime(), _time, _state == PlaybackState::PLAYING);
}

void PlayableEffect::Release()
{
	_gen.Remove(_id);
}

void PlayableEffect::reset()
{
	_time = 0;
	_lastExecutedEffect = _effects.begin();
	_gen.Remove(_id);
}

void PlayableEffect::pause()
{
	_gen.Pause(_id);
}

void PlayableEffect::resume() {
	_gen.Resume(_id);
}



EventVisitor::EventVisitor(float time):m_time(time)
{

}

EventExecutor::EventExecutor(boost::uuids::uuid & id, HapticEventGenerator& basicGen):
	m_id(id), 
	m_basicHapticGenerator(basicGen)
{
}

void EventExecutor::operator()(BasicHapticEvent & h)
{
	m_basicHapticGenerator.NewEvent(AreaFlag(h.Area), h.Duration, h.Effect, h.Strength, m_id);
}

TotalPlaytimeVisitor::TotalPlaytimeVisitor():
	m_totalPlaytime(0), 
	m_fudgeFactor(0.25f)
{
}

float TotalPlaytimeVisitor::TotalPlaytime()
{
	return m_totalPlaytime;
}
