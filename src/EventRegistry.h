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

	void UnregisterEventDriver(const std::string& region, const std::shared_ptr<HardwareDriver>& driver) {
		m_eventRegions.Remove(region, driver);
	}

	void UnregisterRtpDriver(const std::string& region, const std::shared_ptr<HardwareDriver>& driver) {
		m_rtpRegions.Remove(region, driver);
	}

	 const std::vector<std::shared_ptr<HardwareDriver>>* GetEventDrivers(const std::string& region) {
		return m_eventRegions.Find(region); //try to make const
	}

	 const std::vector<std::shared_ptr<HardwareDriver>>* GetRtpDrivers(const std::string& region) {
		return m_rtpRegions.Find(region); //try to make const
	}
private:
	RegionTree m_rtpRegions;
	RegionTree m_eventRegions;
};