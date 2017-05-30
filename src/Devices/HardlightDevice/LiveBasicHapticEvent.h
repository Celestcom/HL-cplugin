#pragma once
#include <boost/uuid/uuid.hpp>

class BasicHapticEventData {
public:
	uint32_t effect;
	uint32_t area;
	float duration;
	float strength;
};

class LiveBasicHapticEvent {
public:
	LiveBasicHapticEvent();
	LiveBasicHapticEvent(boost::uuids::uuid parentId, boost::uuids::uuid uniqueId, BasicHapticEventData data);
	const BasicHapticEventData& Data() const;
	bool operator==(const LiveBasicHapticEvent& other) const;
	void update(float dt);
	bool isFinished() const;
	bool isContinuous() const;
	bool isOneshot() const;
	bool isChildOf(const boost::uuids::uuid& parentId) const;
	boost::uuids::uuid GetParentId();
private:
	boost::uuids::uuid parentId;
	boost::uuids::uuid uniqueId;
	float currentTime;
	bool isPlaying;
	BasicHapticEventData eventData;
};