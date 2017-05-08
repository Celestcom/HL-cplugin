#include "stdafx.h"
#include "HardwareDriver.h"

bool operator<(const HardwareDriver & lhs, const HardwareDriver & rhs)
{
	return lhs.Id() < rhs.Id();
}
