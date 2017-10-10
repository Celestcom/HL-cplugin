#pragma once
#include <stdint.h>

#include "HLVR.h"

#define INTERNAL_TOOL
#ifdef INTERNAL_TOOL
	#ifdef HLVR_EXPORTS
	#define HLVR_UTIL_API __declspec(dllexport) 
	#else
	#define HLVR_UTIL_API __declspec(dllimport) 
	#endif
#else 
	#define HLVR_UTIL_API
#endif
#define HLVR_RETURN_UTIL(ReturnType) HLVR_UTIL_API ReturnType __cdecl

#ifdef __cplusplus
extern "C" {
#endif


	

	HLVR_RETURN_UTIL(int) NSVR_Util_BasicHapticEvent_Create(HLVR_Timeline* timeline,
		float time,
		float strength,
		float duration,
		uint32_t effect,
		uint32_t* regions, 
		unsigned int regionsCount
		)
	{
		HLVR_EventData* event;
		HLVR_EventData_Create(&event, HLVR_EventDataType_SimpleHaptic);
		HLVR_EventData_SetFloat(event, HLVR_EventDataKey_Time_Float, time);
		HLVR_EventData_SetFloat(event, HLVR_EventDataKey_SimpleHaptic_Strength_Float, strength);
		HLVR_EventData_SetFloat(event, HLVR_EventDataKey_SimpleHaptic_Duration_Float, duration);
		HLVR_EventData_SetUInt32s(event, HLVR_EventDataKey_SimpleHaptic_Where_Regions_UInt32s, regions, regionsCount);
		HLVR_EventData_SetInt(event, HLVR_EventDataKey_SimpleHaptic_Effect_Int, effect);

		HLVR_Timeline_AddEvent(timeline, event);

		HLVR_EventData_Destroy(&event);

		return HLVR_Ok;

	}

	

#ifdef __cplusplus
}
#endif

