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


	

	NSVR_RETURN_INTERNAL(int) NSVR_Util_BasicHapticEvent_Create(NSVR_Timeline* timeline,
		float time,
		float strength,
		float duration,
		uint32_t effect,
		uint32_t* regions, 
		unsigned int regionsCount
		)
	{
		NSVR_Event* event;
		NSVR_Event_Create(&event, NSVR_EventType_SimpleHaptic);
		NSVR_Event_SetFloat(event, NSVR_EventKey_Time_Float, time);
		NSVR_Event_SetFloat(event, NSVR_EventKey_SimpleHaptic_Strength_Float, strength);
		NSVR_Event_SetFloat(event, NSVR_EventKey_SimpleHaptic_Duration_Float, duration);
		NSVR_Event_SetUInt32s(event, NSVR_EventKey_SimpleHaptic_Regions_UInt32s, regions, regionsCount);
		NSVR_Event_SetInt(event, NSVR_EventKey_SimpleHaptic_Effect_Int, effect);

		NSVR_Timeline_AddEvent(timeline, event);

		NSVR_Event_Release(&event);

		return NSVR_Success_Unqualified;

	}

	

#ifdef __cplusplus
}
#endif

