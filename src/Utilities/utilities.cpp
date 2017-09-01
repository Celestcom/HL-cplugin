#include "stdafx.h"
#include "utilities.h"
#include "PathFinder.h"




NSVR_RETURN(NSVR_Result) NSVR_Util_Strike(NSVR_Timeline * inTimeline, NSVR_Util_StrikeParams* params)
{
	static PathFinder search;
	auto path = search.ShortestPath(static_cast<PathFinder::named_region>(params->FromRegion), static_cast<PathFinder::named_region>(params->ToRegion));

	float offsetDelta = params->TotalDuration / path.size();

	NSVR_Event* event;
	NSVR_Event_Create(&event, NSVR_EventType_BasicHapticEvent);

	for (std::size_t i = 0; i < path.size(); i++) {
		
		NSVR_Event_SetFloat(event, "time", i * offsetDelta);
		NSVR_Event_SetFloat(event, "duration", 0.0);
		uint32_t region = static_cast<uint32_t>(path[i]);

		NSVR_Event_SetUInt32s(event, "area", &region, 1);
		NSVR_Event_SetInt(event, "effect", params->Effect);

		NSVR_Timeline_AddEvent(inTimeline, event);
	}

	NSVR_Event_Release(&event);

	return NSVR_Success_Unqualified;
}
