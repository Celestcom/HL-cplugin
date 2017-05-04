#pragma once
#include <unordered_set>
#include "HapticDriver.h"

class HapticRequest {
public:
	virtual void accept(HapticDriver& driver) const = 0;
};

class EventRequest : public HapticRequest {
private:
	void accept(HapticDriver& driver) const override;
};

class RTPRequest : public HapticRequest {
private:
	 void accept(HapticDriver& driver) const override;

};
class DriverSet {
	public:
		enum class DriverType {
			BasicHapticEvent = 1
		};
		void AddDriver(DriverType, HapticDriver driver);
		void HandleRequest(DriverType whichDriver, const HapticRequest& request) {
			if (m_drivers.find(whichDriver) != m_drivers.end()) {
				request.accept(m_drivers.at(whichDriver));
			}
		}
private:
	std::unordered_map<DriverType, HapticDriver> m_drivers;
};