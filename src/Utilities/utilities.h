#pragma once

#include "../Plugin/NSLoader.h"


typedef struct NSVR_Util_StrikeParams {
	uint32_t FromRegion;
	uint32_t ToRegion;
	NSVR_Effect Effect;
	float TotalDuration;
} NSVR_Util_StrikeParams;

NSVR_RETURN(NSVR_Result) NSVR_Util_Strike(NSVR_Timeline* inTimeline, NSVR_Util_StrikeParams* params);