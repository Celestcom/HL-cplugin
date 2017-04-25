#include "StdAfx.h"
#include "HapticsPlayer.h"
#include "HapticEvent.h"
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>
#include "PlayableEffect.h"
using namespace std;


HapticsPlayer::HapticsPlayer():
	_model(), 
	_paused(false),
	_generator(_model)
{
	
}

HapticsPlayer::~HapticsPlayer()
{
}

void HapticsPlayer::Play(HapticHandle hh)
{
	std::lock_guard<std::mutex> guard(m_effectsMutex);

	if (auto effect = toInternalUUID(hh)) {
		effect.get()->Play();
	}
}

void HapticsPlayer::Pause(HapticHandle hh)
{
	std::lock_guard<std::mutex> guard(m_effectsMutex);

	if (auto effect = toInternalUUID(hh)) {
		effect.get()->Pause();
	}
}

void HapticsPlayer::Restart(HapticHandle hh)
{
	std::lock_guard<std::mutex> guard(m_effectsMutex);

	if (auto effect = toInternalUUID(hh)) {
		NS::Playable::Restart(effect.get());
	}
}

void HapticsPlayer::Stop(HapticHandle hh)
{
	std::lock_guard<std::mutex> guard(m_effectsMutex);

	if (auto effect = toInternalUUID(hh)) {
		effect.get()->Stop();
	}
}


void HapticsPlayer::Release(HapticHandle hh)
{
	std::lock_guard<std::mutex> guard(m_effectsMutex);
	

	//std::cout << "Got a new handle to release\n";

	auto h = _outsideHandleToUUID[hh];
	
	auto it = _effects.find(uuid_hasher(h));
	if (it != _effects.end()) {
		_releasedEffects.push_back(Released(h));
	}
	else {
		//std::cout << "Tried to release a handle that I never had in the first place\n";
	}

	_outsideHandleToUUID.erase(_outsideHandleToUUID.find(hh));
}

void HapticsPlayer::Create(HapticHandle h, std::vector<SuitEvent> decoded)
{
	std::lock_guard<std::mutex> guard(m_effectsMutex);

	if (_outsideHandleToUUID.find(h) != _outsideHandleToUUID.end()) {
		auto id = _outsideHandleToUUID[h];
		_effects[uuid_hasher(id)]->Stop();
		_effects[uuid_hasher(id)] = std::unique_ptr<IPlayable>(new PlayableEffect(std::move(decoded), _generator, _uuidGen));

	}
	else {
		auto id = _uuidGen();

		_outsideHandleToUUID[h] = id;

		_effects[uuid_hasher(id)] = std::unique_ptr<IPlayable>(new PlayableEffect(std::move(decoded), _generator, _uuidGen));
	}

}

PriorityModel & HapticsPlayer::GetModel()
{
	return _model;
}

int HapticsPlayer::NumLiveEffects()
{
	return _effects.size();
}

int HapticsPlayer::NumOrphanedEffects()
{
	return _releasedEffects.size();
}





bool EffectIsExpired(const std::unique_ptr<IPlayable> &p, bool isGlobalPause) {
	const auto& effectInfo = p->GetInfo();
	return !effectInfo.Playing();

	/*
	if (isGlobalPause) {
		return effectInfo.CurrentTime() >= effectInfo.Duration();
		//return p->CurrentTime() >= p->GetTotalPlayTime();
	}
	else {
		return !effectInfo.Playing() || effectInfo.CurrentTime() >= effectInfo.Duration();
		return !p->IsPlaying() || p->CurrentTime() >= p->GetTotalPlayTime();
	}
	*/
}
int compute(int p) {
	return p * 123123;
}
std::vector<NullSpaceIPC::EffectCommand> HapticsPlayer::Update(float dt)
{
	m_effectsMutex.lock();

	
	for (auto& effect : _effects) {
		effect.second->Update(dt);
	}

	//mark & erase from _effects


	for (auto& released : _releasedEffects) {
		auto hashed_id = uuid_hasher(released.ID);
		auto it = _effects.find(hashed_id);
		if (it != _effects.end()) {
			if (EffectIsExpired(it->second, _paused)) {
				released.NeedsSweep = true;
				it->second->Release();
				_effects.erase(it);
			}
		}
		else {
			BOOST_LOG_TRIVIAL(error) << "[HapticsPlayer] Tried to release an effect, but couldn't find it!";

		}
		
	}


	//sweep from _released

	auto toRemove = std::remove_if(_releasedEffects.begin(), _releasedEffects.end(), [](const Released& e) {
		return e.NeedsSweep;
	});
	_releasedEffects.erase(toRemove, _releasedEffects.end());

	m_effectsMutex.unlock();
	///BUGG!!!! Try running the graph engine at full speed. Locking error?
	return _model.Update(dt);
}


void HapticsPlayer::PlayAll()
{
	std::lock_guard<std::mutex> guard(m_effectsMutex);

	_paused = false;

	
	//take anything that was frozen (not simply paused by the user) and resume it
	for (auto& effect : _effects) {
		if (std::find(_frozenEffects.begin(), _frozenEffects.end(), effect.first) != _frozenEffects.end()) {
			effect.second->Play();
		}
	}

	_frozenEffects.clear();
}
//frozen vs paused

void HapticsPlayer::PauseAll()
{
	std::lock_guard<std::mutex> guard(m_effectsMutex);

	_paused = true;


	for (auto& effect : _effects) {
		if (effect.second->IsPlaying()) {
			effect.second->Pause();
			_frozenEffects.push_back(effect.first);
		}
	}
}

void HapticsPlayer::ClearAll()
{
	std::lock_guard<std::mutex> guard(m_effectsMutex);

	for (auto& effect : _effects) {
		effect.second->Stop();
	}
	_outsideHandleToUUID.clear();
	_effects.clear();
	_releasedEffects.clear();
}






boost::optional<const std::unique_ptr<IPlayable>&> HapticsPlayer::toInternalUUID(HapticHandle hh)
{
	auto h = uuid_hasher(_outsideHandleToUUID[hh]);
	if (_effects.find(h) != _effects.end()) {
		return _effects.at(h);
	}
	
	return boost::optional<const std::unique_ptr<IPlayable>&>();
}
