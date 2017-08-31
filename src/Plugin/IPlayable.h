#pragma once
#include <unordered_map>
#include "boost\uuid\uuid.hpp"

struct PlayableInfo {
private:
	float _totalPlaytime;
	float _currentTime;
	int _state;
public:
	float Duration() const {
		return _totalPlaytime;
	}
	float CurrentTime() const {
		return _currentTime;
	}
	int State() const {
		return _state;
	}
	PlayableInfo(float duration, float currentTime, int playing) :
		_totalPlaytime(duration),
		_currentTime(currentTime),
		_state(playing) {}
};


class IPlayable {
public:
	virtual ~IPlayable() = default;
	//Should begin playback of an effect if it has not started, else should resume from paused
	virtual void Play() = 0;

	//If paused or playing, shall return to initial state and not continue playing
	virtual void Stop() = 0;

	//Shall only have an effect when playing, which is to pause playback
	virtual void Pause() = 0;

	//Cleanup resources associated with the effect, pending removal by the HapticsPlayer
	virtual void Release() = 0;


	//Update playback of effect if playing, given delta time in fractional seconds
	virtual void Update(float dt) = 0;
	
	//Check if this effect has already been released
	virtual bool IsReleased() const = 0;

	//Return info about the playable
	virtual PlayableInfo GetInfo() const = 0;

	//Return the total length of the effect in fractional seconds
	virtual float GetTotalDuration() const = 0;

	//Return the current time of the effect in fractional seconds
	virtual float CurrentTime() const = 0;

	//Returns true only if the effect is playing
	virtual bool IsPlaying() const = 0;
};

