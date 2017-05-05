#pragma once
#include "RegionTree.h"

class EventRegistry {
public:
	EventRegistry();
	void RegisterEventDriver(const std::string& region, std::shared_ptr<HardwareDriver> driver) {
		m_eventRegions.Insert(region, driver);
	}

	void RegisterRtpDriver(const std::string& region, std::shared_ptr<HardwareDriver> driver) {
		m_rtpRegions.Insert(region, driver);
	}

	const std::vector<std::shared_ptr<HardwareDriver>>* GetEventDrivers(const std::string& region) {
		return m_eventRegions.Find(region); //try to make const
	}

	const std::vector<std::shared_ptr<HardwareDriver>>* GetRtptDrivers(const std::string& region) {
		return m_rtpRegions.Find(region); //try to make const
	}
private:
	RegionTree m_rtpRegions;
	RegionTree m_eventRegions;
};