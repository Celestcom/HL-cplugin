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



	int getNodeRegion(uint32_t nodeIndex, uint32_t * outRegion)
	{
		*outRegion = pairs[nodeIndex].Region;
		return NSVR_Success_Unqualified;
	}



	int getIntensity(uint32_t nodeIndex, float * outIntensity)
	{
		//perhaps be defensive and reject request if wrong type
		*outIntensity = pairs[nodeIndex].Value.data_0;
		return NSVR_Success_Unqualified;
	}



	int getNodeColor(uint32_t nodeIndex, NSVR_Color * outColor)
	{
		const auto& col = pairs[nodeIndex].Value;
		//this may not work. 
		//todo: test

		*outColor = { col.data_0, col.data_1, col.data_2, col.data_3 };
		/*outColor->r = col.r;
		outColor->g = col.g;
		outColor->b = col.b;
		outColor->a = col.a;*/
		return NSVR_Success_Unqualified;
	}


};