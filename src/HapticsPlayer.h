#pragma once
#include <boost\uuid\uuid.hpp>
#include <boost\uuid\random_generator.hpp>

#include "HapticClasses.h"
#include "IPlayable.h"
#include "PriorityModel.h"
#include "HapticEventGenerator.h"
#include "SuitEvent.h"
#include <mutex>

typedef unsigned int HapticHandle;

//todo: all misnamed now; must be a generic EventPlayer
class HapticsPlayer
{
public:
	struct Released {
	public:
		boost::uuids::uuid ID;
		bool NeedsSweep;
		Released(boost::uuids::uuid id) :ID(id), NeedsSweep(false) {}
	};

	
	HapticsPlayer();
	~HapticsPlayer();

	std::vector<NullSpaceIPC::EffectCommand> Update(float dt);
	void Play(HapticHandle h);
	void Pause(HapticHandle h);
	void Restart(HapticHandle h);
	void Stop(HapticHandle h);
	void Release(HapticHandle h);
	void Create(HapticHandle h, std::vector<SuitEvent>);

	PriorityModel& GetModel();

	std::size_t NumLiveEffects();
	std::size_t NumOrphanedEffects();

	void PlayAll();
	void PauseAll();
	void ClearAll();


	std::vector<std::pair<AreaFlag, uint16_t>> GetIntensities() const;
	
private:
	boost::hash<boost::uuids::uuid> uuid_hasher;
	boost::uuids::random_generator _uuidGen;

	std::mutex m_effectsMutex;
	std::unordered_map<std::size_t, std::unique_ptr<IPlayable>> _effects;

	PriorityModel _model; //order dependency (1)
	HapticEventGenerator _generator; //order dependency (2)

	std::vector<Released> _releasedEffects;
	std::unordered_map<HapticHandle, boost::uuids::uuid> _outsideHandleToUUID;
	std::vector<std::size_t> _frozenEffects;

	bool _paused;

	
	boost::optional<const std::unique_ptr<IPlayable>&>  toInternalUUID(HapticHandle hh);
};

