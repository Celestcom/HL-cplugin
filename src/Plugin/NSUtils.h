#pragma once
#include <stdint.h>

#include "NSLoader.h"

#define INTERNAL_TOOL
#ifdef INTERNAL_TOOL
	#ifdef NSLOADER_EXPORTS
	#define NSLOADER_INTERNAL_API __declspec(dllexport) 
	#else
	#define NSLOADER_INTERNAL_API __declspec(dllimport) 
	#endif
#else 
	#define NSLOADER_INTERNAL_API
#endif
#define NSVR_RETURN_INTERNAL(ReturnType) NSLOADER_INTERNAL_API ReturnType __cdecl

#ifdef __cplusplus
extern "C" {
#endif


	NSVR_RETURN_INTERNAL(int) NSVR_Util_Curve_Create(NSVR_Timeline* timeline, float time, float* time_offsets, float* mags, unsigned int length) {
		NSVR_Event* event;
		//should be RealtimeEvent or something     ---v
		NSVR_Event_Create(&event, NSVR_EventType_CurveHapticEvent);

		for (unsigned int i = 0; i < length; i++) {
			NSVR_Event_SetFloat(event, "magnitude", mags[i]);
			NSVR_Event_SetFloat(event, "time", time_offsets[i]);
			NSVR_Timeline_AddEvent(timeline, event);
		}

		NSVR_Event_Release(&event);

		return NSVR_Success_Unqualified;
	}

	NSVR_RETURN_INTERNAL(int) NSVR_Util_BasicHapticEvent_Create(NSVR_Timeline* timeline,
		float time,
		float strength,
		float duration,
		uint32_t area,
		uint32_t effect)
	{
		NSVR_Event* event;
		NSVR_Event_Create(&event, NSVR_EventType_BasicHapticEvent);
		NSVR_Event_SetFloat(event, "time", time);
		NSVR_Event_SetFloat(event, "strength", strength);
		NSVR_Event_SetFloat(event, "duration", duration);
		NSVR_Event_SetInt(event, "area", area);
		NSVR_Event_SetInt(event, "effect", effect);

		NSVR_Timeline_AddEvent(timeline, event);

		NSVR_Event_Release(&event);

		return NSVR_Success_Unqualified;

	}


	//NSVR_Timeline* coolEffect;

	//float times[16];
	//float mags[16];
	//
	//NSVR_Curve_Create(timeline, 0.0, &times, &mags, 16);
	//
	//
	//
	//
	//
	//
	//
	//
	

#ifdef __cplusplus
}
#endif

