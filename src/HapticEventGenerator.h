#pragma once
#include <memory>
#include <unordered_map>

#include <boost\uuid\uuid.hpp>
#include <boost\optional\optional.hpp>
#include <boost\functional\hash.hpp>
#include "Enums.h"
#include "HapticEvent.h"
#include "PriorityModel.h"
class HapticEventGenerator
{
public:
	HapticEventGenerator(PriorityModel& model);
	~HapticEventGenerator();
	void NewEvent(AreaFlag area, float duration, uint32_t effect, float strength, boost::uuids::uuid id);
	void Resume(boost::uuids::uuid id);
	void Pause(boost::uuids::uuid id);
	void Remove(boost::uuids::uuid id);
private:
	struct GeneratedEvent {
		HapticEvent Event;
		AreaFlag Area;
		bool Paused;
		bool NeedsDeletion;
		GeneratedEvent(HapticEvent e, AreaFlag a) : Event(e), Area(a), Paused(false), NeedsDeletion(false) {}
		GeneratedEvent() :Event(), Area(AreaFlag::None), Paused(), NeedsDeletion(false) {}
	};
	PriorityModel& _model;
	std::unordered_map<std::size_t, std::vector<GeneratedEvent>> _events;
	boost::hash<boost::uuids::uuid> uuid_hasher;

};

