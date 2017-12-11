#pragma once

#include "SharedTypes.h"
struct BodyView {
	BodyView() :pairs{} {}
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
		//perhaps be defensive and reject request if wrong type
		*outIntensity = pairs[nodeIndex].Value.data_0;
		return HLVR_Ok;
	}

	int getWaveform(uint32_t nodeIndex, int32_t* outWaveform) const  {
		*outWaveform = static_cast<int>(pairs[nodeIndex].Value.data_1);
		return HLVR_Ok;
	}





};