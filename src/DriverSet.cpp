#include "stdafx.h"
#include "DriverSet.h"

void DriverSet::AddDriver(DriverType d, HapticDriver driver)
{
	m_drivers[d] = driver;
}

void EventRequest::accept(HapticDriver & driver) const
{
	driver.visit(*this);
}

void RTPRequest::accept(HapticDriver & driver) const
{
	driver.visit(*this);
}
