#include "stdafx.h"
#include "LiveBasicHapticEvent.h"



LiveBasicHapticEvent::LiveBasicHapticEvent() :
	parentId(),
	uniqueId(),
	currentTime(0),
	isPlaying(false),
	eventData()
{
}

LiveBasicHapticEvent::LiveBasicHapticEvent(boost::uuids::uuid parentId, boost::uuids::uuid uniqueId, BasicHapticEventData data) :
	parentId(parentId),
	uniqueId(uniqueId),
	currentTime(0),
	isPlaying(true),
	eventData(std::move(data))

{

}

const BasicHapticEventData & LiveBasicHapticEvent::Data() const
{
	return eventData;
}

bool LiveBasicHapticEvent::operator==(const LiveBasicHapticEvent & other) const
{
	return uniqueId == other.uniqueId;
}

void LiveBasicHapticEvent::update(float dt)
{
	if (isPlaying) {
		currentTime += dt;
	}
}

bool LiveBasicHapticEvent::isFinished() const
{
	return currentTime >= eventData.duration;
}

bool LiveBasicHapticEvent::isContinuous() const
{
	return !isOneshot();
}

bool LiveBasicHapticEvent::isOneshot() const
{
	return eventData.duration == 0.0;
}

bool LiveBasicHapticEvent::isChildOf(const boost::uuids::uuid & parentId) const
{
	return this->parentId == parentId;
}

boost::uuids::uuid LiveBasicHapticEvent::GetParentId()
{
	return parentId;
}