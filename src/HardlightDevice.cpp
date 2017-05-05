#include "stdafx.h"
#include "HardlightDevice.h"
#include "NSLoader.h"

GeneratedEvent::GeneratedEvent(boost::uuids::uuid id, std::unique_ptr<IRetainedEvent> event) : id(id), event(std::move(event)) 
{

}

bool GeneratedEvent::operator==(const GeneratedEvent & other)
{
	
	return id == other.id;
	
}

HardlightDevice::HardlightDevice()
{
	
}

void HardlightDevice::RegisterDrivers(EventRegistry& registry)
{
	registry.RegisterEventDriver("chest_left", chestLeft);
}

void HardlightDevice::UnregisterDrivers(EventRegistry& registry)
{
	//remove event drivers
}
