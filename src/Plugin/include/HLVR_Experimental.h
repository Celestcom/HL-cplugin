#pragma once
#include <stdint.h>
#include "HLVR_Forwards.h"
#include "HLVR_Errors.h"

#define INTERNAL_TOOL
#ifdef INTERNAL_TOOL
	#ifdef HLVR_EXPORTS
	#define HLVR_EXPERIMENTAL_API __declspec(dllexport) 
	#else
	#define HLVR_EXPERIMENTAL_API __declspec(dllimport) 
	#endif
#else 
	#define HLVR_EXPERIMENTAL_API
#endif
#define HLVR_RETURN_EXP(ReturnType) HLVR_EXPERIMENTAL_API ReturnType __cdecl

#ifdef __cplusplus
extern "C" {
#endif





	/* Immediate API */
	typedef struct HLVR_BodyView HLVR_BodyView;


	HLVR_RETURN_EXP(HLVR_Result) HLVR_Immediate_Set(HLVR_System* systemPtr, uint32_t* regions, double* amplitudes, uint32_t length);

	HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_Create(HLVR_BodyView** body);
	HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_Release(HLVR_BodyView** body);
	HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_Poll(HLVR_BodyView* body, HLVR_System* system);
	HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_GetNodeCount(HLVR_BodyView* body, uint32_t* outNodeCount);
	HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_GetNodeType(HLVR_BodyView * body, uint32_t nodeIndex, uint32_t* outType);
	HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_GetNodeRegion(HLVR_BodyView * body, uint32_t nodeIndex, uint32_t* outRegion);

	HLVR_RETURN_EXP(HLVR_Result) HLVR_BodyView_GetIntensity(HLVR_BodyView * body, uint32_t nodeIndex, float* outIntensity);


	/* Tracking */
	HLVR_RETURN_EXP(HLVR_Result) HLVR_System_PollTracking(HLVR_System* ptr, HLVR_TrackingUpdate* updatePtr);
	HLVR_RETURN_EXP(HLVR_Result) HLVR_System_EnableTracking(HLVR_System* ptr);
	HLVR_RETURN_EXP(HLVR_Result) HLVR_System_DisableTracking(HLVR_System* ptr);


#ifdef __cplusplus
}
#endif

