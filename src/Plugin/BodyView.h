#pragma once

#include "SharedTypes.h"

// BodyView is the user-facing interface to the suit's visualizer.
// It's just a struct containing a list of <region, type, id, data>, which was retrieved from 
// shared memory. The reason that it's an object is because it allows the user to do something like:
// OpaqueBodyView p = Bodyview_Create();
// Bodyview_Poll(p);
// float strength = Bodyview_Intensity(whatever);
// So it is only updated when they poll.


struct BodyView {
	BodyView() : pairs{} {}
	std::vector<NullSpace::SharedMemory::RegionPair> pairs;
	
	int getNodeType(uint32_t nodeIndex, uint32_t * outType) const
	{
		*outType = pairs[nodeIndex].Type;
		return HLVR_Ok;
	}

	int getNodeRegion(uint32_t nodeIndex, uint32_t * outRegion) const
	{
		*outRegion = pairs[nodeIndex].Region;
		return HLVR_Ok;
	}

	int getIntensity(uint32_t nodeIndex, float * outIntensity) const
	{
		*outIntensity = pairs[nodeIndex].Value.data_0;
		return HLVR_Ok;
	}

	int getWaveform(uint32_t nodeIndex, int32_t* outWaveform) const  {
		*outWaveform = static_cast<int>(pairs[nodeIndex].Value.data_1);
		return HLVR_Ok;
	}

};