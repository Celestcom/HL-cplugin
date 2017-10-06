#pragma once

#include "HLVR_Errors.h"

#include <stdint.h>

#ifdef HLVR_EXPORTS
#define HLVR_API __declspec(dllexport) 
#else
#define HLVR_API __declspec(dllimport) 
#endif

#define HLVR_API_VERSION_MAJOR 0
#define HLVR_API_VERSION_MINOR 4
#define HLVR_API_VERSION_PATCH 0
#define HLVR_API_VERSION ((HLVR_API_VERSION_MAJOR << 24) | (HLVR_API_VERSION_MINOR << 16) | HLVR_API_VERSION_PATCH)


#define HLVR_RETURN(ReturnType) HLVR_API ReturnType __cdecl



#ifdef __cplusplus
extern "C" {
#endif
	static const int32_t hlvr_int32max = 0x7FFFFFFF;
	static const int32_t hlvr_int32min = -hlvr_int32max - 1;

	typedef enum HLVR_EventKey {
		HLVR_EventKey_UNKNOWN = 0,
		/* Common keys*/
		HLVR_EventKey_Time_Float,

		/* Event-Specific keys*/
		HLVR_EventKey_SimpleHaptic_Duration_Float = 1000,
		HLVR_EventKey_SimpleHaptic_Strength_Float,
		HLVR_EventKey_SimpleHaptic_Effect_Int,
		HLVR_EventKey_SimpleHaptic_Regions_UInt32s,
		HLVR_EventKey_SimpleHaptic_Nodes_UInt32s,

		HLVR_EventKey_MIN = hlvr_int32min,
		HLVR_EventKey_MAX = hlvr_int32max

	} HLVR_EventKey;
	

	const int32_t HLVR_SUBREGION_BLOCK = 1000000;

	typedef enum HLVR_Region {
		hlvr_region_UNKNOWN = 0,
		hlvr_region_body = 1 * HLVR_SUBREGION_BLOCK,
		hlvr_region_torso = 2 * HLVR_SUBREGION_BLOCK,
		hlvr_region_torso_front = 3 * HLVR_SUBREGION_BLOCK,
		hlvr_region_chest_left = 4 * HLVR_SUBREGION_BLOCK,
		hlvr_region_chest_right = 5 * HLVR_SUBREGION_BLOCK,
		hlvr_region_upper_ab_left = 6 * HLVR_SUBREGION_BLOCK,
		hlvr_region_middle_ab_left = 7 * HLVR_SUBREGION_BLOCK,
		hlvr_region_lower_ab_left = 8 * HLVR_SUBREGION_BLOCK,
		hlvr_region_upper_ab_right = 9 * HLVR_SUBREGION_BLOCK,
		hlvr_region_middle_ab_right = 10 * HLVR_SUBREGION_BLOCK,
		hlvr_region_lower_ab_right = 11 * HLVR_SUBREGION_BLOCK,
		hlvr_region_torso_back = 12 * HLVR_SUBREGION_BLOCK,
		hlvr_region_torso_left = 13 * HLVR_SUBREGION_BLOCK,
		hlvr_region_torso_right = 14 * HLVR_SUBREGION_BLOCK,
		hlvr_region_upper_back_left = 15 * HLVR_SUBREGION_BLOCK,
		hlvr_region_upper_back_right = 16 * HLVR_SUBREGION_BLOCK,
		hlvr_region_upper_arm_left = 17 * HLVR_SUBREGION_BLOCK,
		hlvr_region_lower_arm_left = 18 * HLVR_SUBREGION_BLOCK,
		hlvr_region_upper_arm_right = 19 * HLVR_SUBREGION_BLOCK,
		hlvr_region_lower_arm_right = 20 * HLVR_SUBREGION_BLOCK,
		hlvr_region_shoulder_left = 21 * HLVR_SUBREGION_BLOCK,
		hlvr_region_shoulder_right = 22 * HLVR_SUBREGION_BLOCK,
		hlvr_region_upper_leg_left = 23 * HLVR_SUBREGION_BLOCK,
		hlvr_region_lower_leg_left = 24 * HLVR_SUBREGION_BLOCK,
		hlvr_region_upper_leg_right = 25 * HLVR_SUBREGION_BLOCK,
		hlvr_region_lower_leg_right = 26 * HLVR_SUBREGION_BLOCK,
		hlvr_region_head = 27 * HLVR_SUBREGION_BLOCK,
		hlvr_region_palm_left = 28 * HLVR_SUBREGION_BLOCK,
		hlvr_region_palm_right = 29 * HLVR_SUBREGION_BLOCK,
		hlvr_region_MIN = hlvr_int32min,
		hlvr_region_MAX = hlvr_int32max
	} HLVR_Region;

	
	typedef struct HLVR_Agent HLVR_Agent;

	typedef struct HLVR_Event HLVR_Event;

	typedef struct HLVR_Timeline HLVR_Timeline;

	typedef struct HLVR_Effect HLVR_Effect;

	
	
	typedef enum HLVR_Waveform {
		HLVR_Waveform_UNKNOWN = 0,
		HLVR_Waveform_Bump = 1,
		HLVR_Waveform_Buzz = 2,
		HLVR_Waveform_Click = 3,
		HLVR_Waveform_Fuzz = 5,
		HLVR_Waveform_Hum = 6,
		HLVR_Waveform_Pulse = 8,
		HLVR_Waveform_Tick = 11,
		HLVR_Waveform_Double_Click = 4,
		HLVR_Waveform_Triple_Click = 16,
		HLVR_Waveform_MIN = hlvr_int32min,
		HLVR_Waveform_MAX = hlvr_int32max
	} HLVR_Waveform;

	typedef struct HLVR_Quaternion {
		float w;
		float x;
		float y;
		float z;
	} HLVR_Quaternion;


	typedef struct HLVR_TrackingUpdate {
		HLVR_Quaternion chest;
		HLVR_Quaternion left_upper_arm;
		HLVR_Quaternion left_forearm;
		HLVR_Quaternion right_upper_arm;
		HLVR_Quaternion right_forearm;
	} HLVR_TrackingUpdate;



	typedef enum HLVR_EventType {
		HLVR_EventType_UNKNOWN = 0,
		HLVR_EventType_SimpleHaptic = 1,
		HLVR_EventType_MIN = hlvr_int32min,
		HLVR_EventType_MAX = hlvr_int32max
	} HLVR_EventType;

	typedef enum HLVR_DeviceConcept {
		HLVR_DeviceConcept_UNKNOWN,
		HLVR_DeviceConcept_Suit,
		HLVR_DeviceConcept_Controller,
		HLVR_DeviceConcept_Headwear,
		HLVR_DeviceConcept_Gun,
		HLVR_DeviceConcept_Sword,
		HLVR_DeviceConcept_MIN = hlvr_int32min,
		HLVR_DeviceConcept_MAX = hlvr_int32max
	} HLVR_DeviceConcept;

	typedef enum HLVR_DeviceStatus {
		HLVR_DeviceStatus_Unknown = 0,
		HLVR_DeviceStatus_Connected = 1,
		HLVR_DeviceStatus_Disconnected = 2
	} HLVR_DeviceStatus;

	typedef struct HLVR_DeviceInfo {
		uint32_t Id;
		char Name[128];
		HLVR_DeviceConcept Concept;
		HLVR_DeviceStatus Status;
		
	} HLVR_DeviceInfo;


	typedef enum HLVR_NodeConcept {
		NSVR_NodeConcept_UNKNOWN = 0,
		HLVR_NodeConcept_Haptic,
		HLVR_NodeConcept_LED,
		HLVR_NodeConcept_InertialTracker,
		HLVR_NodeConcept_AbsoluteTracker,
		HLVR_NodeConcept_MIN = hlvr_int32min,
		HLVR_NodeConcept_MAX = hlvr_int32max
	} HLVR_NodeConcept;

	typedef struct HLVR_NodeInfo {
		uint32_t Id;
		char Name[128];
		HLVR_NodeConcept Concept;

	} HLVR_NodeInfo;
	typedef struct HLVR_DeviceIterator {
		void* _internal;
		HLVR_DeviceInfo DeviceInfo;
	} HLVR_DeviceIterator;

	typedef struct HLVR_NodeIterator {
		void* _internal;
		HLVR_NodeInfo NodeInfo;
	} HLVR_NodeIterator;

	typedef struct HLVR_PlatformInfo {
		uint32_t MajorVersion;
		uint32_t MinorVersion;
	} HLVR_PlatformInfo;

	typedef enum HLVR_EffectInfo_State {
		HLVR_EffectInfo_State_Unknown = 0,
		HLVR_EffectInfo_State_Playing,
		HLVR_EffectInfo_State_Paused,
		HLVR_EffectInfo_State_Idle
	} HLVR_EffectInfo_State;

	typedef struct HLVR_EffectInfo {
		float Duration;
		float Elapsed;
		HLVR_EffectInfo_State PlaybackState;
	} HLVR_EffectInfo;
	
	
	typedef struct HLVR_AgentConfiguration HLVR_AgentConfiguration;


	HLVR_RETURN(HLVR_Result) HLVR_DeviceIterator_Init(HLVR_DeviceIterator* iter);
	HLVR_RETURN(HLVR_Result) HLVR_DeviceIterator_Next(HLVR_DeviceIterator* iter, HLVR_Agent* system);

	HLVR_RETURN(HLVR_Result) HLVR_NodeIterator_Init(HLVR_NodeIterator* iter);
	HLVR_RETURN(HLVR_Result) HLVR_NodeIterator_Next(HLVR_NodeIterator* iter, uint32_t device_id, HLVR_Agent* system);

	//Instantiates a new NSVR system context
	HLVR_RETURN(HLVR_Result) HLVR_Agent_Create(HLVR_Agent** systemPtr, HLVR_AgentConfiguration* config);

	//Returns true if this API supports the feature specified by a null-terminated string
	HLVR_RETURN(int) HLVR_Version_HasFeature(const char* feature);

	//Returns the version of this plugin, in the format ((Major << 24) | (Minor << 16) | Patch)
	HLVR_RETURN(uint32_t) HLVR_Version_Get(void);
	
	//Returns true if the plugin is compatible with this header, false otherwise
	HLVR_RETURN(int) HLVR_Version_IsCompatibleDLL(void);

	
	HLVR_RETURN(void) HLVR_Agent_Destroy(HLVR_Agent** ptr);

	


	HLVR_RETURN(HLVR_Result) HLVR_Agent_GetPlatformInfo(HLVR_Agent* systemPtr, HLVR_PlatformInfo* infoPtr);


	/* Haptics engine */ 
	HLVR_RETURN(HLVR_Result) HLVR_Agent_SuspendEffects(HLVR_Agent* ptr);
	HLVR_RETURN(HLVR_Result) HLVR_Agent_ResumeEffects(HLVR_Agent* ptr);
	HLVR_RETURN(HLVR_Result) HLVR_Agent_CancelEffects(HLVR_Agent* ptr);




	
	
	/* Events */
	HLVR_RETURN(HLVR_Result) HLVR_Event_Create(HLVR_Event** eventPtr, HLVR_EventType type);
	HLVR_RETURN(void)		 HLVR_Event_Destroy(HLVR_Event** event);

	HLVR_RETURN(HLVR_Result) HLVR_Event_SetFloat(HLVR_Event* event, HLVR_EventKey key, float value);
	HLVR_RETURN(HLVR_Result) HLVR_Event_SetFloats(HLVR_Event* event, HLVR_EventKey key, float* values, unsigned int length);

	HLVR_RETURN(HLVR_Result) HLVR_Event_SetInt(HLVR_Event* event, HLVR_EventKey key, int value);
	HLVR_RETURN(HLVR_Result) HLVR_Event_SetInts(HLVR_Event* event, HLVR_EventKey key, int* array, unsigned int length);

	HLVR_RETURN(HLVR_Result) HLVR_Event_SetUInt32(HLVR_Event* event, HLVR_EventKey key, uint32_t value);
	HLVR_RETURN(HLVR_Result) HLVR_Event_SetUInt32s(HLVR_Event * event, HLVR_EventKey key, uint32_t* array, unsigned int length);

	HLVR_RETURN(HLVR_Result) HLVR_Event_SetUInt64(HLVR_Event * event, HLVR_EventKey key, uint64_t value);
	HLVR_RETURN(HLVR_Result) HLVR_Event_SetUInt64s(HLVR_Event * event, HLVR_EventKey key, uint64_t* array, unsigned int length);



	// Timelines 
	HLVR_RETURN(HLVR_Result) HLVR_Timeline_Create(HLVR_Timeline** eventListPtr);
	HLVR_RETURN(void)		 HLVR_Timeline_Destroy(HLVR_Timeline** listPtr);
	HLVR_RETURN(HLVR_Result) HLVR_Timeline_AddEvent(HLVR_Timeline* list, HLVR_Event* event);
	HLVR_RETURN(HLVR_Result) HLVR_Timeline_Transmit(HLVR_Timeline* timeline, HLVR_Agent* systemPtr, HLVR_Effect* handlePr);



	// Effect Playback 
	HLVR_RETURN(HLVR_Result) HLVR_Effect_Create(HLVR_Effect** handlePtr);
	HLVR_RETURN(void)		 HLVR_Effect_Destroy(HLVR_Effect** handlePtr);
	HLVR_RETURN(HLVR_Result) HLVR_Effect_Play(HLVR_Effect* handle);
	HLVR_RETURN(HLVR_Result) HLVR_Effect_Pause(HLVR_Effect* handle);
	HLVR_RETURN(HLVR_Result) HLVR_Effect_Reset(HLVR_Effect* handle);
	HLVR_RETURN(HLVR_Result) HLVR_Effect_GetInfo(HLVR_Effect* effect, HLVR_EffectInfo* info);






#ifdef __cplusplus
}
#endif

