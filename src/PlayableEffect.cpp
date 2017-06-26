#include "stdafx.h"
#include "PlayableEffect.h"
#include "Locator.h"
#include <iostream>
#include "BasicHapticEvent.h"
#include "NSLoader.h"
#include <memory>
#include <iterator>
#include <numeric>
#include <bitset>
#include <array>

#include <functional>
#include <chrono>


#include "ClientMessenger.h"
#pragma warning(push)
#pragma warning(disable : 4267)
#include "HighLevelEvent.pb.h"
#pragma warning(pop)

template<typename T>
T time(std::function<void()> fn) {
	auto then = std::chrono::high_resolution_clock::now();
	fn();
	auto now = std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now() - then);
	return now;
}


PlayableEffect::PlayableEffect(std::vector<PlayablePtr>&& effects,EventRegistry& reg, boost::uuids::random_generator& uuid, ClientMessenger& messenger) :
	m_effects(std::move(effects)),
	m_state(PlaybackState::IDLE),
	m_registry(reg),
	m_id(uuid()),
	m_time(0),
	m_released(false),
	m_messenger(messenger)
{
	assert(!m_effects.empty());

	sortByTime(m_effects);

	//It is unclear if this de-duplication should happen at all, or if it should
	//happen at a higher level. It feels wrong to iterate over thousands of duplicates when
	//the lower level will be forced to wipe them out. 

	pruneDuplicates(m_effects);

	reset();
}

void PlayableEffect::sortByTime(std::vector<PlayablePtr>& playables)
{
	std::sort(playables.begin(), playables.end(), cmp_by_time);
}


void PlayableEffect::pruneDuplicates(std::vector<PlayablePtr>& playables) {
	
	auto last = std::unique(playables.begin(), playables.end(), cmp_by_duplicate);
	playables.erase(last, playables.end());
	playables.shrink_to_fit();
}



PlayableEffect::PlayableEffect(PlayableEffect && rhs) :
	m_effects(std::move(rhs.m_effects)),
	m_state(rhs.m_state),
	m_registry(rhs.m_registry),
	m_id(rhs.m_id),
	m_time(rhs.m_time),
	m_released(rhs.m_released),
	m_activeDrivers(rhs.m_activeDrivers),
	m_lastExecutedEffect(m_effects.begin()),
	m_messenger(rhs.m_messenger)
{
	
}

PlayableEffect::~PlayableEffect()
{
	
}

void PlayableEffect::Play()
{
	switch (m_state) {
	case PlaybackState::IDLE:
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

NullSpaceIPC::HighLevelEvent makeEvent(const boost::uuids::uuid& parentId, const PlayablePtr& event) {
	


	using namespace NullSpaceIPC;
	HighLevelEvent abstract_event;

	//Right now, we are using UUID. We shouldn't be truncating a UUID like I am below.
	//Todo: generate our own random IDs, say, from 0 to 2^64 - 1.
	//The other option which I have tried is to send over a byte array representing the full
	//UUID. I think this is overkill, and it means we need to pass it on the hardware plugins as well.
	uint64_t truncatedId;
	memcpy_s(&truncatedId, sizeof(truncatedId), parentId.data, sizeof uint64_t);
	
	abstract_event.set_parent_id(truncatedId);
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
			
			using namespace NullSpaceIPC;
			
			HighLevelEvent event = makeEvent(m_id, *current);

			std::vector<std::string> regions =  extractRegions(*current);
			
			for (const auto& region : regions) {

				event.set_region(region);
				m_messenger.WriteEvent(event);
				//auto consumers = m_registry.GetEventDrivers(region);
				//if (consumers) {
				//	//need translator from registry's leaves to area flags for backwards compat?
				//	std::for_each(consumers->begin(), consumers->end(), [&](auto& consumer) {
				//		assert(current->get()->area() != 0);
				//			consumer->createRetained(m_id, *current);
				//		m_activeDrivers.insert(std::weak_ptr<HardwareDriver>(consumer));
				//	});
				//}
			}
				
			std::advance(current, 1);
		}
		else {
			//precondition: this requires that the effects vector was sorted by time.
			//Given that, we can stop here because if it wasn't expired, then the next won't be either
			break;
		}
	}

	m_lastExecutedEffect = current;

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
	return m_released;
}

PlayableInfo PlayableEffect::GetInfo() const
{
	return PlayableInfo(GetTotalDuration(), m_time, m_state == PlaybackState::PLAYING);
}

void PlayableEffect::Release()
{
	m_released = true;

	
}


NullSpaceIPC::HighLevelEvent makePlaybackEvent(const boost::uuids::uuid& parentId, NullSpaceIPC::PlaybackEvent::Command command) {
	using namespace NullSpaceIPC;
	HighLevelEvent event;

	uint64_t truncatedId;
	memcpy_s(&truncatedId, sizeof(truncatedId), parentId.data, sizeof uint64_t);

	event.set_parent_id(truncatedId);

	PlaybackEvent* playback_event = event.mutable_playback_event();
	playback_event->set_command(command);
	return event;
}
void PlayableEffect::reset()
{
	m_time = 0;
	m_lastExecutedEffect = m_effects.begin();
	m_messenger.WriteEvent(makePlaybackEvent(m_id, NullSpaceIPC::PlaybackEvent_Command_CANCEL));
}

void PlayableEffect::pause()
{
	m_messenger.WriteEvent(makePlaybackEvent(m_id, NullSpaceIPC::PlaybackEvent_Command_PAUSE));
}

void PlayableEffect::resume() {
	m_messenger.WriteEvent(makePlaybackEvent(m_id, NullSpaceIPC::PlaybackEvent_Command_UNPAUSE));
}



RegionVisitor::RegionVisitor()
{
}


std::array<std::string, 32> regionmap = {

	"left_forearm",
	"left_upper_arm",
	"left_shoulder",
	"left_back",
	"left_upper_chest",
	"left_upper_ab",
	"left_mid_ab",
	"left_lower_ab",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"right_forearm",
	"right_upper_arm",
	"right_shoulder",
	"right_back",
	"right_upper_chest",
	"right_upper_ab",
	"right_mid_ab",
	"right_lower_ab",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved"
};
	


std::vector<std::string> extractRegions(const PlayablePtr & event) 
{
	auto& translator = Locator::getTranslator();
	std::vector<std::string> regions;
	uint32_t area = event->area();
	std::bitset<32> areas(area);
	for (std::size_t i = 0; i < areas.size(); i++) {
		if (areas.test(i)) {
			regions.push_back(regionmap[i]);
		}
	}
	/*
	for (uint32_t bit = 1; area >= bit; bit *= 2)
	{
		if (area & bit) {
			switch (AreaFlag(bit)) {
			case AreaFlag::Forearm_Left:
				regions.push_back(translator.ToRegionString(AreaFlag::Forearm_Left));
				break;
			case AreaFlag::Upper_Arm_Left:
				regions.push_back(translator.ToRegionString(AreaFlag::Upper_Arm_Left));
				break;
			case AreaFlag::Shoulder_Left:
				regions.push_back(translator.ToRegionString(AreaFlag::Shoulder_Left));
				break;
			case AreaFlag::Back_Left:
				regions.push_back(translator.ToRegionString(AreaFlag::Back_Left));
				break;
			case AreaFlag::Chest_Left:
				regions.push_back(translator.ToRegionString(AreaFlag::Chest_Left));
				break;
			case AreaFlag::Upper_Ab_Left:
				regions.push_back(translator.ToRegionString(AreaFlag::Upper_Ab_Left));
				break;
			case AreaFlag::Mid_Ab_Left:
				regions.push_back(translator.ToRegionString(AreaFlag::Mid_Ab_Left));
				break;
			case AreaFlag::Lower_Ab_Left:
				regions.push_back(translator.ToRegionString(AreaFlag::Lower_Ab_Left));
				break;
			case AreaFlag::Forearm_Right:
				regions.push_back(translator.ToRegionString(AreaFlag::Forearm_Right));
				break;
			case AreaFlag::Upper_Arm_Right:
				regions.push_back(translator.ToRegionString(AreaFlag::Upper_Arm_Right));
				break;
			case AreaFlag::Shoulder_Right:
				regions.push_back(translator.ToRegionString(AreaFlag::Shoulder_Right));
				break;
			case AreaFlag::Back_Right:
				regions.push_back(translator.ToRegionString(AreaFlag::Back_Right));
				break;
			case AreaFlag::Chest_Right:
				regions.push_back(translator.ToRegionString(AreaFlag::Chest_Right));
				break;
			case AreaFlag::Upper_Ab_Right:
				regions.push_back(translator.ToRegionString(AreaFlag::Upper_Ab_Right));
				break;
			case AreaFlag::Mid_Ab_Right:
				regions.push_back(translator.ToRegionString(AreaFlag::Mid_Ab_Right));
				break;
			case AreaFlag::Lower_Ab_Right:
				regions.push_back(translator.ToRegionString(AreaFlag::Lower_Ab_Right));
				break;
			default:
				break;
			}
		}
	}
	*/
	return regions;
}

