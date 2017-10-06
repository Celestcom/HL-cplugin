#pragma once

#include <stdint.h>

typedef int32_t HLVR_Result;


#ifndef HLVR_OK
#define HLVR_OK(result) (result >= 0)
#endif


#ifndef HLVR_FAIL
#define HLVR_FAIL(result) (!HLVR_OK(result))
#endif



typedef enum HLVR_SuccessTypes {
	HLVR_Ok = 0,
	
	/* Polling data */
	HLVR_Ok_NoDataAvailable = 1000,

	
} HLVR_SuccessTypes;

typedef enum HLVR_ErrorTypes {


	/* General errors*/
	HLVR_Error_Unknown = -1000,
	HLVR_Error_NullArgument = -1001,
	HLVR_Error_InvalidArgument = -1002,

	
	/* Creation errors */
	HLVR_Error_InvalidEventType = -2000,


	/* Platform connection errors*/
	HLVR_Error_NotConnected = -4000,

	/* Handle errors */
	HLVR_Error_NoSuchHandle = -5000,

	/*Iteration */
	HLVR_Error_NoMoreDevices = -6000,
	HLVR_Error_NoMoreNodes = -7000



} HLVR_ErrorTypes;

