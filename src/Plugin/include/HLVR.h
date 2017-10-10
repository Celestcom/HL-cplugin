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

	/*! \enum HLVR_EventDataKey
		\brief Used to specify which property to modify on an HLVR_EventData. 
	*/
	typedef enum HLVR_EventDataKey {
		HLVR_EventDataKey_UNKNOWN = 0,
		/* Required keys*/

		/* Event-Specific keys*/
		HLVR_EventDataKey_SimpleHaptic_Duration_Float = 1000,	//optional, defaults to 0 (natural waveform duration)
		HLVR_EventDataKey_SimpleHaptic_Strength_Float,			//optional, default to 1.0
		HLVR_EventDataKey_SimpleHaptic_Effect_Int,				//optional, defaults to HLVR_Waveform_Click
		HLVR_EventDataKey_SimpleHaptic_Where_Regions_UInt32s,	//The 'Where' key can either be specified as a list of 1 or more regions,
		HLVR_EventDataKey_SimpleHaptic_Where_Nodes_UInt32s,		//or a list of 1 or more nodes. Defaults to hlvr_region_UNKNOWN.

		HLVR_EventDataKey_MIN = hlvr_int32min,
		HLVR_EventDataKey_MAX = hlvr_int32max

	} HLVR_EventDataKey;
	

	const int32_t HLVR_SUBREGION_BLOCK = 1000000;
	/*! \enum HLVR_Region
		\brief A set of one ore more HLVR_Regions are used to specify where an HLVR_EventData should take place. 
	
		When higher fidelity devices are supported, more named regions will be added for convenience.
		
	*/
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

	
	/*! \struct HLVR_System
		\brief Opaque type representing the main context object used in the HLVR API. 
		
		The HLVR_System is responsible for forwarding your requests to Hardlight-compatible hardware, providing information about
		what devices and nodes are registered in the system, and delivering data such as tracking streams and device connection information.

		Unless otherwise noted, calls into the Hardlight API are only valid while the corresponding HLVR_System object is alive. 
	*/
	typedef struct HLVR_System HLVR_System;

	/*! \struct HLVR_EventData
		\brief Opaque type representing a parameterizable event, used to create various effects on Hardlight-compatible hardware.

		A commonly used HLVR_EventData is the SimpleHaptic, which specifies a haptic event to be played on a particular set of hardware-defined nodes or regions.
		Strength, duration, waveform, and location may be specified differently and then combined in an HLVR_Timeline to form a complete effect. 
	*/
	typedef struct HLVR_EventData HLVR_EventData;

	/*! \struct HLVR_Timeline
		\brief Opaque type representing the fundamental time-ordered data source for HLVR_Effects. 

		HLVR_Timelines are containers used to group together one or more HLVR_EventDatas. Multiple different HLVR_EventDataTypes can exist within the same timeline. 
	*/
	typedef struct HLVR_Timeline HLVR_Timeline;

	/*! \struct HLVR_Effect
		\brief Opaque type representing a playable effect, such as a haptic traversal of the body. 

		An HLVR_Effect is essentially a handle, which can be manipulated to start, stop, reset, and retrieve info about an effect within the system. 
		
	*/
	typedef struct HLVR_Effect HLVR_Effect;

	
	/*!	\enum HLVR_Waveform
		\brief Fundamental waveforms which may be combined to form more complex units of haptic feedback. 
	*/
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

	/*! \struct HLVR_Quaternion
		\brief Represents a quaternion in the order w, x, y, z.
	*/
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


	/*! \enum HLVR_EventDataType
		\brief Used to specify which type of HLVR_EventData should be created.
	*/
	typedef enum HLVR_EventType {
		HLVR_EventType_UNKNOWN = 0,
		HLVR_EventType_SimpleHaptic = 1,
		HLVR_EventType_MIN = hlvr_int32min,
		HLVR_EventType_MAX = hlvr_int32max
	} HLVR_EventType;

	/*!	\enum HLVR_DeviceConcept 
		\brief Used to give a general description of a particular device connected to the system.
	*/
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

	/*! \enum HLVR_DeviceStatus 
		\brief Specifies if a device has a confirmed connection to the system
	*/
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

	/*!	\enum HLVR_NodeConcept
	\brief Used to give a general description of a particular node connected to the system.
	*/
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

	/*! \struct HLVR_DeviceIterator
		\brief Use HLVR_DeviceIterator and associated calls to enumerate all devices recognized by the system. 

		When iterating over devices, the current device's info will be stored in the DeviceInfo member. 

		If the HLVR_DeviceIterator is not initialized with HLVR_DeviceIterator_Init, or you manually modify _internal,
		behavior is undefined.

		This struct has no associated Destroy call and can be declared on the stack. 
	*/
	typedef struct HLVR_DeviceIterator {
		void* _internal;
		HLVR_DeviceInfo DeviceInfo;
	} HLVR_DeviceIterator;


	/*! \struct HLVR_NodeIterator
		\brief Use HLVR_NodeIterator and associated calls to enumerate all nodes within a particular device recognized by the system.

		When iterating over nodes, the current node's info will be stored in the NodeInfo member.

		If the HLVR_NodeIterator is not initialized with HLVR_NodeIterator_Init, or you manually modify _internal,
		behavior is undefined.

		This struct has no associated Destroy call and can be declared on the stack.
	*/
	typedef struct HLVR_NodeIterator {
		void* _internal;
		HLVR_NodeInfo NodeInfo;
	} HLVR_NodeIterator;

	typedef struct HLVR_PlatformInfo {
		uint32_t MajorVersion;
		uint32_t MinorVersion;
	} HLVR_PlatformInfo;

	/*!	\enum HLVR_EffectInfo_State
		\brief The playback state of an existing HLVR_Effect

		When first instantiated in the system by way of HLVR_Timeline_Transmit, an HLVR_Effect will be in the Idle state.
	*/
	typedef enum HLVR_EffectInfo_State {
		HLVR_EffectInfo_State_Unknown = 0,
		HLVR_EffectInfo_State_Playing,
		HLVR_EffectInfo_State_Paused,
		HLVR_EffectInfo_State_Idle
	} HLVR_EffectInfo_State;

	/*!	\struct HLVR_EffectInfo
		\brief HLVR_EffectInfo stores basic information about an effect including total duration, playback state, and elapsed time.

		Duration and Elapsed are in fractional seconds.

		To check if an effect has completed playback, test if Elapsed >= Duration. 
	*/
	typedef struct HLVR_EffectInfo {
		float Duration;
		float Elapsed;
		HLVR_EffectInfo_State PlaybackState;
	} HLVR_EffectInfo;
	
	
	typedef struct HLVR_SystemConfiguration HLVR_SystemConfiguration;

	//Unless otherwise noted, all API calls will return HLVR_Error_NullArgument if a required argument is nullptr, and HLVR_Error_Unknown if an unknown error occurs.
	//Unless otherwise noted, all API calls must be initiated from the same thread. 

	
	/*! Create a new Hardlight API context, establishing communicating between this application and the Hardlight Service.  
		@param config currently ignored
		@return HLVR_Ok if the context was created successfully
	*/
	HLVR_RETURN(HLVR_Result) HLVR_System_Create(HLVR_System** system, HLVR_SystemConfiguration* config);

	/*! Destroy a context, shutting down communication between this application and the Hardlight Service 
		@return HLVR_Ok if the context was destroyed successfully
	*/
	HLVR_RETURN(void) HLVR_System_Destroy(HLVR_System** ptr);

	/*! Checks if this version of the API has a particular feature 
		@param feature name of feature
		@return 1 if has feature, else 0
	*/
	HLVR_RETURN(int) HLVR_Version_HasFeature(const char* feature);

	/*! Retrieves the version of this API in the format (major << 24) | (minor << 16) | patch
		@return uint32_t representing version of the API
	*/
	HLVR_RETURN(uint32_t) HLVR_Version_Get(void);
	
	/*! Checks if this header file is compatible (i.e. same major version) with the supplied library
		@return 1 if compatible, else 0
	*/
	HLVR_RETURN(int) HLVR_Version_IsCompatibleDLL(void);


	/*! Retrieves information about the current connection to the Service.
		@param system current context
		@param info struct to be populated
		@return HLVR_Ok if able to retrieve information, else HLVR_Error_NotConnected if not connected to Service.
	*/
	HLVR_RETURN(HLVR_Result) HLVR_System_GetPlatformInfo(HLVR_System* system, HLVR_PlatformInfo* info);


	/*! Suspend execution of the effects engine, useful within a game pause routine
		@param system current context
		@return HLVR_Ok on success
	*/
	HLVR_RETURN(HLVR_Result) HLVR_System_SuspendEffects(HLVR_System* system);

	/*! Resume execution of the effects engine, useful within a game resume routine
		Only effects that were paused due to a call to HLVR_System_SuspendEffects will be resumed
		@param system current context
		@return HLVR_Ok on success
	*/
	HLVR_RETURN(HLVR_Result) HLVR_System_ResumeEffects(HLVR_System* system);

	/*! Cancel all effects, invalidating all HLVR_Effect handles, useful within a game quit routine or a level change 
		where new HLVR_Effects will be used
		@param system current context
		@return HLVR_Ok on success
	*/
	HLVR_RETURN(HLVR_Result) HLVR_System_CancelEffects(HLVR_System* system);



	/*! Initialize an HLVR_DeviceIterator
		@param iter to an existing HLVR_DeviceIterator.
	*/
	HLVR_RETURN(HLVR_Result) HLVR_DeviceIterator_Init(HLVR_DeviceIterator* iter);

	/*! Advance the iterator to the next device in the system
		@param iter to an existing HLVR_DeviceIterator
		@param system to the current context
		@return HLVR_Ok if a device was retrieved, HLVR_Error_NoMoreDevices if the iterator is finished.
	*/
	HLVR_RETURN(HLVR_Result) HLVR_DeviceIterator_Next(HLVR_DeviceIterator* iter, HLVR_System* system);


	/*! Initialize an HLVR_NodeIterator
		@param iter pointer to an existing HLVR_NodeIterator.
	*/
	HLVR_RETURN(HLVR_Result) HLVR_NodeIterator_Init(HLVR_NodeIterator* iter);

	/*! Advance the iterator to the next node in the system
		@param iter to an existing HLVR_NodeIterator
		@param device_id the device from which to retrieve retrieve nodes. If special ID 0 is supplied,
		all nodes in the system will be enumerated. If an unknown device id is supplied, the iterator will return HLVR_Error_NoMoreNodes.
		@param current context
		@return HLVR_Ok if a node was retrieved, HLVR_Error_NoMoreNodes if the iterator is finished.
	*/
	HLVR_RETURN(HLVR_Result) HLVR_NodeIterator_Next(HLVR_NodeIterator* iter, uint32_t device_id, HLVR_System* system);

	
	
	/*! Create a new event with the given event type
		@param type which type of event to create
		@return 
	*/
	HLVR_RETURN(HLVR_Result) HLVR_EventData_Create(HLVR_EventData** event);
	HLVR_RETURN(void)		 HLVR_EventData_Destroy(HLVR_EventData** event);

	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetFloat(HLVR_EventData* event, HLVR_EventDataKey key, float value);
	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetFloats(HLVR_EventData* event, HLVR_EventDataKey key, float* values, unsigned int length);

	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetInt(HLVR_EventData* event, HLVR_EventDataKey key, int value);
	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetInts(HLVR_EventData* event, HLVR_EventDataKey key, int* array, unsigned int length);

	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetUInt32(HLVR_EventData* event, HLVR_EventDataKey key, uint32_t value);
	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetUInt32s(HLVR_EventData * event, HLVR_EventDataKey key, uint32_t* array, unsigned int length);

	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetUInt64(HLVR_EventData * event, HLVR_EventDataKey key, uint64_t value);
	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetUInt64s(HLVR_EventData * event, HLVR_EventDataKey key, uint64_t* array, unsigned int length);

	//should put in Debug.h
	typedef enum HLVR_EventData_KeyParseError {
		HLVR_EventData_KeyParseError_UNKNOWN,
		HLVR_EventData_KeyParseError_KeyRequired,
		HLVR_EventData_KeyParseError_InvalidValue,
		HLVR_EventData_KeyParseError_WrongValueType,
		HLVR_EventData_KeyParseError_MIN = hlvr_int32min,
		HLVR_EventData_KeyParseError_MAX = hlvr_int32max
	} HLVR_EventData_KeyParseError;

	typedef struct HLVR_EventData_KeyParseResult {
		HLVR_EventDataKey Key;
		HLVR_EventData_KeyParseError Error;

	} HLVR_EventData_KeyParseResult;
	typedef struct HLVR_EventData_ValidationResult {
		int Count;
		HLVR_EventData_KeyParseResult Errors[32];
	} HLVR_EventData_ValidationResult;

	HLVR_RETURN(HLVR_Result) HLVR_EventData_Validate(HLVR_EventData* event, HLVR_EventType type, HLVR_EventData_ValidationResult* outResult);

	//HLVR_Timeline_AddEvent(float timeOffsetSeconds, HLVR_EventDataData* data, HLVR_EventDataType type)
	// Timelines 
	HLVR_RETURN(HLVR_Result) HLVR_Timeline_AddEvent(HLVR_Timeline* timeline, float timeOffsetSeconds, HLVR_EventData* data, HLVR_EventType dataType); 

	HLVR_RETURN(HLVR_Result) HLVR_Timeline_Create(HLVR_Timeline** eventListPtr);
	HLVR_RETURN(void)		 HLVR_Timeline_Destroy(HLVR_Timeline** listPtr);
//	HLVR_RETURN(HLVR_Result) HLVR_Timeline_AddEvent(HLVR_Timeline* list, HLVR_EventData* event);
	HLVR_RETURN(HLVR_Result) HLVR_Timeline_Transmit(HLVR_Timeline* timeline, HLVR_System* systemPtr, HLVR_Effect* handlePr);



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

