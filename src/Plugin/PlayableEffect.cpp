#include "stdafx.h"

#include "PlayableEffect.h"
#include "Locator.h"
#include "ClientMessenger.h"

#pragma warning(push)
#pragma warning(disable : 4267)
#include "HighLevelEvent.pb.h"
#pragma warning(pop)

#include "HLVR.h"
#include <numeric> //std::accumulate






PlayableEffect::PlayableEffect(std::vector<PlayablePtr> effects, boost::uuids::uuid uuid, ClientMessenger& messenger) 
	: m_state(PlaybackState::IDLE)
	, m_time(0.f) //fractional seconds, e.g. 1.5 is one and one half of a second. We should make this a type.
	, m_effects(std::move(effects))
	, m_id(std::move(uuid))
	, m_messenger(messenger)
	, m_isReleased(false)
{
	assert(!m_effects.empty());

	sortByTime(m_effects);

	//It is unclear if this de-duplication should happen at all, or if it should
	//happen at a higher level. It feels wrong to iterate over thousands of duplicates when
	//the lower level will be forced to wipe them out. 

	pruneDuplicates(m_effects);

	scrubToBegin();
}

//Not sure why the move constructor is deleted, but here is a replacement.
//Todo: find out why we can't use the default move constructor.
//I believe it would behave correctly - although we are stealing an iterator from the other object, it would point into the 
//stolen effects vector so all would be good. 
PlayableEffect::PlayableEffect(PlayableEffect && rhs) :
	m_effects(std::move(rhs.m_effects)),
	m_state(rhs.m_state),
	m_id(rhs.m_id),
	m_time(rhs.m_time),
	m_isReleased(rhs.m_isReleased),
	m_lastExecutedEffect(m_effects.begin()),
	m_messenger(rhs.m_messenger)
{

}

void PlayableEffect::sortByTime(std::vector<PlayablePtr>& playables)
{
	auto by_time = [](const auto& lhs, const auto& rhs) { 
		return lhs->time() < rhs->time(); 
	};
	std::sort(playables.begin(), playables.end(), by_time);
}


void PlayableEffect::pruneDuplicates(std::vector<PlayablePtr>& playables) {
	
	auto value_equality = [](const auto& lhs, const auto& rhs) {
		return *lhs == *rhs;
	};

 	auto last = std::unique(playables.begin(), playables.end(), value_equality);
	playables.erase(last, playables.end());
	playables.shrink_to_fit();
}


void PlayableEffect::Play()
{
	switch (m_state) {
	case PlaybackState::IDLE:
		scrubToBegin();
		m_state = PlaybackState::PLAYING;
		break;
	case PlaybackState::PAUSED:
		resume();
		m_state = PlaybackState::PLAYING;
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
	switch (m_state) {
		case PlaybackState::IDLE:
			//remain in idle state
			break;
		case PlaybackState::PAUSED:
			reset();
			m_state = PlaybackState::IDLE;
			break;
		case PlaybackState::PLAYING:
			reset();
			m_state = PlaybackState::IDLE;
			break;
		default:
			break;
	}
}

void PlayableEffect::Pause()
{
	switch (m_state) {
	case PlaybackState::IDLE:
		//remain in idle state
		break;
	case PlaybackState::PAUSED:
		//remain in paused state
		break;
	case PlaybackState::PLAYING:
		pause();
		m_state = PlaybackState::PAUSED;
		break;
	default:
		break;
	}
	
	
}

//Right now, we are using UUID. We shouldn't be truncating a UUID like I am below.
//Todo: generate our own random IDs, say, from 0 to 2^64 - 1.
//The other option which I have tried is to send over a byte array representing the full
//UUID. I think this is overkill, and it means we need to pass it on the hardware plugins as well.
uint64_t truncatedUuid(const boost::uuids::uuid& uuid) {
	uint64_t truncatedId = 0;
	memcpy_s(&truncatedId, sizeof(truncatedId), uuid.data, sizeof uint64_t);
	return truncatedId;
}

NullSpaceIPC::HighLevelEvent makeEvent(const boost::uuids::uuid& parentId, const PlayablePtr& event) {
	
	using namespace NullSpaceIPC;
	HighLevelEvent abstract_event;

	abstract_event.set_parent_id(truncatedUuid(parentId));
	event->serialize(abstract_event);
	return abstract_event;
}

void PlayableEffect::Update(float dt)
{
	if (m_state == PlaybackState::IDLE || m_state == PlaybackState::PAUSED) {
		return;
	}

	m_time += dt;
	
	auto current(m_lastExecutedEffect);

	auto isTimeExpired = [this](const PlayablePtr& event) {
		return event->time() <= m_time;
	};

	
	while (current != m_effects.end()) {
		if (isTimeExpired(*current)) {
			NullSpaceIPC::HighLevelEvent event = makeEvent(m_id, *current);			
			m_messenger.WriteEvent(event);
			std::advance(current, 1);
		}
		else {
			//Precondition: effects vector must be sorted by time (which we do in the constructor)
			//Given that, we can stop here because if this effect wasn't expired, then the next won't be either
			break;
		}
	}

	m_lastExecutedEffect = current;

	//Automatically stop when we exceed the total duration of the effect
	if (m_time >= GetTotalDuration()) {
		Stop();
	}

} 


float PlayableEffect::GetTotalDuration() const
{
	return std::accumulate(m_effects.begin(), m_effects.end(), 0.0f, [](float currentDuration, const auto& effect) {
		float thisEffectEndTime = std::max(0.0f, effect->duration() + effect->time());
		return std::max(currentDuration, thisEffectEndTime);
	});
}

float PlayableEffect::CurrentTime() const
{
	return m_time;
}

bool PlayableEffect::IsPlaying() const
{
	return m_state == PlaybackState::PLAYING;
}

bool PlayableEffect::IsReleased() const
{
	return m_isReleased;
}

EffectInfo PlayableEffect::GetInfo() const
{
	return EffectInfo{ GetTotalDuration(), m_time, (int)m_state };
}

void PlayableEffect::Release()
{
	m_isReleased = true;	
}


NullSpaceIPC::HighLevelEvent makePlaybackEvent(const boost::uuids::uuid& parentId, NullSpaceIPC::PlaybackEvent::Command command) {
	using namespace NullSpaceIPC;
	HighLevelEvent event;

	event.set_parent_id(truncatedUuid(parentId));

	PlaybackEvent* playback_event = event.mutable_playback_event();
	playback_event->set_command(command);
	return event;
}


void PlayableEffect::scrubToBegin()
{
	m_time = 0;
	m_lastExecutedEffect = m_effects.begin();
}

void PlayableEffect::reset()
{
	m_messenger.WriteEvent(makePlaybackEvent(m_id, NullSpaceIPC::PlaybackEvent_Command_CANCEL));
}

void PlayableEffect::pause()
{
	m_messenger.WriteEvent(makePlaybackEvent(m_id, NullSpaceIPC::PlaybackEvent_Command_PAUSE));
}

void PlayableEffect::resume() {
	m_messenger.WriteEvent(makePlaybackEvent(m_id, NullSpaceIPC::PlaybackEvent_Command_UNPAUSE));
}



