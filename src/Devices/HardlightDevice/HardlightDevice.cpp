#include "stdafx.h"
#include "HardlightDevice.h"
#include "NSLoader.h"
#include "Locator.h"
#include "SuitEvent.h"
#include <boost/uuid/random_generator.hpp>
#include <experimental/vector>






HardlightDevice::HardlightDevice() 
{
	for (int loc = (int)Location::Lower_Ab_Right; loc != (int)Location::Error; loc++) {
		m_drivers.push_back(std::make_shared<Hardlight_Mk3_ZoneDriver>(Location(loc)));
	}
}

void HardlightDevice::RegisterDrivers(EventRegistry& registry)
{
	auto& translator = Locator::getTranslator();
	for (auto& driver : m_drivers) {
		auto region = translator.ToRegionString(
			translator.ToArea(driver->Location())
		);

		registry.RegisterEventDriver(region, driver);
		registry.RegisterRtpDriver(region, driver);
	}
	
}

void HardlightDevice::UnregisterDrivers(EventRegistry& registry)
{
	auto& translator = Locator::getTranslator();

	for (auto& driver : m_drivers) {
		auto region = translator.ToRegionString(
			translator.ToArea(driver->Location())
		);

		registry.UnregisterEventDriver(region, driver);
		registry.UnregisterRtpDriver(region, driver);
	}
}

CommandBuffer HardlightDevice::GenerateHardwareCommands(float dt)
{
	CommandBuffer result;
	for (auto& driver : m_drivers) {
		CommandBuffer cl = driver->update(dt);
		//std::reverse(cl.begin(), cl.end()); todo: see if necessary order is already created
		result.insert(result.begin(), cl.begin(), cl.end());
	}
	return result;
		
}

DisplayResults HardlightDevice::QueryDrivers()
{
	DisplayResults representations;

	for (const auto& driver : m_drivers) {
		auto result = driver->QueryCurrentlyPlaying();
		if (result) {
			representations.push_back(*result);
		}
	}

	return representations;
}



ZoneModel::UserCommand::UserCommand(): id(), command(Command::Unknown)
{
}

ZoneModel::UserCommand::UserCommand(boost::uuids::uuid id, Command c) : id(id), command(c)
{
}


