#pragma once

#include "SharedTypes.h"
struct BodyView {
	BodyView() :pairs{} {}
	std::vector<NullSpace::SharedMemory::RegionPair> pairs;
	
	int getNodeType(uint32_t nodeIndex, uint32_t * outType)
	{
		*outType = pairs[nodeIndex].Type;
		return NSVR_Success_Unqualified;
	}



	int getNodeRegion(uint32_t nodeIndex, uint64_t * outRegion)
	{
		*outRegion = pairs[nodeIndex].Region;
		return NSVR_Success_Unqualified;
	}



	int getIntensity(uint32_t nodeIndex, float * outIntensity)
	{
		//perhaps be defensive and reject request if wrong type
		*outIntensity = pairs[nodeIndex].intensity.intensity;
		return NSVR_Success_Unqualified;
	}



	int getNodeColor(uint32_t nodeIndex, NSVR_Color * outColor)
	{
		const auto& col = pairs[nodeIndex].color;
		//this may not work. 
		//todo: test

		*outColor = { col.r, col.g, col.b, col.a };
		/*outColor->r = col.r;
		outColor->g = col.g;
		outColor->b = col.b;
		outColor->a = col.a;*/
		return NSVR_Success_Unqualified;
	}


};