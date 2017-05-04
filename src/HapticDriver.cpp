#include "stdafx.h"
#include "HapticDriver.h"
#include "DriverSet.h"

void EventDriver::visit(const EventRequest & req)
{


}

void RTPDriver::visit(const RTPRequest & req)
{
}

bool RTPDriver::canVisit(const RTPRequest & req)
{
	//return req.area == m_area;
}
