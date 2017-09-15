#pragma once

#include "../Plugin/NSLoader.h"


typedef struct NSVR_Util_StrikeParams {
	uint32_t FromRegion;
	uint32_t ToRegion;
	NSVR_Effect Effect;
	float TotalDuration;
} NSVR_Util_StrikeParams;


typedef struct NSVR_Util_EmanationParams {
	uint32_t FromRegion;
	uint32_t Depth;
	NSVR_Effect Effect;
	float FromStrength;
	float ToStrength;
	float TotalDuration;
	bool Gather;

} NSVR_Util_EmanationParams;
NSVR_RETURN(NSVR_Result) NSVR_Util_Strike(NSVR_Timeline* inTimeline, NSVR_Util_StrikeParams* params);

NSVR_RETURN(NSVR_Result) NSVR_Util_Emanation(NSVR_Timeline* inTimeline, NSVR_Util_EmanationParams* params);


