#pragma once
/*! \file */


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


	

	/*! Special value indicating that an HLVR_NodeIterator should iterate over all nodes in the system.
		@see HLVR_NodeIterator_Next
	*/
	const uint32_t hlvr_allnodes = 0;

	/*! Properties that can be set on an HLVR_EventData. 
		
		When a key has a common prefix, such as Where_Regions and Where_Nodes, it means that only one of keys
		should be specified. A SimpleHaptic can target a particular set of device nodes, or a particular set of regions, but not both.
	*/
	typedef enum HLVR_EventDataKey {
		HLVR_EventDataKey_UNKNOWN = 0,

		/* Event-Specific keys */
		HLVR_EventDataKey_SimpleHaptic_Duration_Float = 1000,	/*!< defaults to 0.0 (natural waveform duration) */
		HLVR_EventDataKey_SimpleHaptic_Strength_Float,			/*!< default to 1.0 */
		HLVR_EventDataKey_SimpleHaptic_Effect_Int,				/*!< defaults to HLVR_Waveform_Click */
		HLVR_EventDataKey_SimpleHaptic_Where_Regions_UInt32s,	/*!< defaults to hlvr_region_UNKNOWN */
		HLVR_EventDataKey_SimpleHaptic_Where_Nodes_UInt32s,		/*!< defaults to using Where_Regions, see previous */

		HLVR_EventDataKey_MIN = hlvr_int32min,
		HLVR_EventDataKey_MAX = hlvr_int32max

	} HLVR_EventDataKey;
	

	const int32_t HLVR_SUBREGION_BLOCK = 1000000;
	
	/*!
		A set of one or more HLVR_Regions are used to specify where particular events should take place. 

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

	
	/*!
		Opaque type representing the main context object used in the HLVR API. 
		
		The HLVR_System is responsible for forwarding your requests to Hardlight-compatible hardware, providing information about
		what devices and nodes are registered in the system, and delivering data such as tracking streams and device connection information.

		Unless otherwise noted, calls into the Hardlight API are only valid while the corresponding HLVR_System object is alive. 
	*/
	typedef struct HLVR_System HLVR_System;

	/*! 
		Opaque type representing a parameterizable event, used to create various effects on Hardlight-compatible hardware.

		A commonly used HLVR_EventData is the SimpleHaptic, which specifies a haptic event to be played on a particular set of hardware-defined nodes or regions.
		Strength, duration, waveform, and location may be specified differently and then combined in an HLVR_Timeline to form a complete effect. 
	*/
	typedef struct HLVR_EventData HLVR_EventData;

	/*! 
		Opaque type representing the fundamental time-ordered data source for HLVR_Effects. 

		HLVR_Timelines are containers used to group together one or more HLVR_EventDatas. Multiple different HLVR_EventDataTypes can exist within the same timeline. 
	*/
	typedef struct HLVR_Timeline HLVR_Timeline;

	/*!
		Opaque type representing a playable effect, such as a haptic traversal of the body. 

		An HLVR_Effect is essentially a handle, which can be manipulated to start, stop, reset, and retrieve info about an effect within the system. 
	*/
	typedef struct HLVR_Effect HLVR_Effect;

	
	/*!
		Fundamental waveforms which may be combined to form more complex units of haptic feedback. 
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

	/*! 
		Represents a quaternion in the order w, x, y, z.
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


	/*!
		Specify which type of type of event should be created when adding to an HLVR_Timeline

		@see HLVR_Timeline_AddEvent
	*/
	typedef enum HLVR_EventType {
		HLVR_EventType_UNKNOWN = 0,
		HLVR_EventType_SimpleHaptic = 1,
		HLVR_EventType_MIN = hlvr_int32min,
		HLVR_EventType_MAX = hlvr_int32max
	} HLVR_EventType;

	/*!
		General description of a particular device connected to the system.
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

	/*
		Confirmed connection status of a device recognized by the system.

		@note If a device was previously connected and is now disconnected physically, the device will still be 
		enumerated by a call to HLVR_DeviceIterator_Next with a Disconnected status
	*/
	typedef enum HLVR_DeviceStatus {
		HLVR_DeviceStatus_UNKNOWN = 0,
		HLVR_DeviceStatus_Connected = 1,
		HLVR_DeviceStatus_Disconnected = 2,
		HLVR_DeviceStatus_MIN = hlvr_int32min,
		HLVR_DeviceStatus_MAX = hlvr_int32max
	} HLVR_DeviceStatus;

	/*!
		General information about a device recognized by the system.
	*/
	typedef struct HLVR_DeviceInfo {
		/*! Unique ID; not necessarily persisted through service restarts */
		uint32_t Id;
		/*! Friendly name  */
		char Name[128];
		/*! General concept */
		HLVR_DeviceConcept Concept;
		/*! Connection status to the host */
		HLVR_DeviceStatus Status;
		
	} HLVR_DeviceInfo;

	/*!
		General description of a particular node connected to the system.
	*/
	typedef enum HLVR_NodeConcept {
		HLVR_NodeConcept_UNKNOWN = 0,
		HLVR_NodeConcept_Haptic,
		HLVR_NodeConcept_LED,
		HLVR_NodeConcept_InertialTracker,
		HLVR_NodeConcept_AbsoluteTracker,
		HLVR_NodeConcept_MIN = hlvr_int32min,
		HLVR_NodeConcept_MAX = hlvr_int32max
	} HLVR_NodeConcept;

	/*!
		General information about a node recognized by the system
	*/
	typedef struct HLVR_NodeInfo {
		/*! Unique ID; not necessarily persisted through service restarts */
		uint32_t Id;
		/*! Friendly name */
		char Name[128];
		/*! General concept */
		HLVR_NodeConcept Concept;
	} HLVR_NodeInfo;

	/*! 
		Use HLVR_DeviceIterator and associated calls to enumerate all devices recognized by the system. 

		When iterating over devices, the current device's info will be stored in the DeviceInfo member. 

		The struct must be initialized with HLVR_DeviceIterator_Init.
	
		This struct has no associated Destroy call and can be declared on the stack. 
	*/
	typedef struct HLVR_DeviceIterator {
		void* _internal;
		HLVR_DeviceInfo DeviceInfo;
	} HLVR_DeviceIterator;


	/*! 
		Use HLVR_NodeIterator and associated calls to enumerate all nodes registered to a particular device recognized by the system.

		When iterating over nodes, the current node's info will be stored in the NodeInfo member.

		The struct must be initialized with HLVR_NodeIterator_Init.

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

	/*!	
		The playback state of an existing HLVR_Effect.

		When first instantiated in the system by way of HLVR_Timeline_Transmit, an HLVR_Effect will be in the Idle state.
	*/
	typedef enum HLVR_EffectInfo_State {
		HLVR_EffectInfo_State_UNKNOWN = 0,
		HLVR_EffectInfo_State_Playing,
		HLVR_EffectInfo_State_Paused,
		HLVR_EffectInfo_State_Idle,
		HLVR_EffectInfo_State_MIN = hlvr_int32min,
		HLVR_EffectInfo_State_MAX = hlvr_int32max
	} HLVR_EffectInfo_State;

	/*!	
		HLVR_EffectInfo stores basic information about an effect including total duration, playback state, and elapsed time.

		To check if an effect has completed playback, test if Elapsed >= Duration. 
	*/
	typedef struct HLVR_EffectInfo {
		/*! Approximate duration in fractional seconds */
		float Duration;
		/*! Approximate elapsed progress in fractional seconds */
		float Elapsed;
		/*! Playback state*/
		HLVR_EffectInfo_State PlaybackState;
	} HLVR_EffectInfo;
	
	
	typedef struct HLVR_SystemConfiguration HLVR_SystemConfiguration;

	//Unless otherwise noted, all API calls will return HLVR_Error_NullArgument if a required argument is nullptr, and HLVR_Error_Unknown if an unknown error occurs.
	//Unless otherwise noted, all API calls must be initiated from the same thread. 

	
	/*! Create a new Hardlight API context, establishing communicating between this application and the Hardlight Service.  
		
		Usage:
		@code
		HLVR_System* system = nullptr;
		if (HLVR_FAIL(HLVR_System_Create(&system, nullptr))) {
			//oh no, we couldn't instantiate the system!
		}
		@endcode

		@param system address of the target pointer
		@param config currently ignored
		@return HLVR_Ok if the context was created successfully
		
	*/
	HLVR_RETURN(HLVR_Result) HLVR_System_Create(HLVR_System** system, HLVR_SystemConfiguration* config);

	/*! Destroy a context, shutting down communication between this application and the Hardlight Service.

		@return HLVR_Ok if the context was destroyed successfully
	*/
	HLVR_RETURN(void) HLVR_System_Destroy(HLVR_System** ptr);

	/*! Checks if this version of the API has a particular feature.
	
		@param feature name of feature
		@return 1 if has feature, else 0
	*/
	HLVR_RETURN(int) HLVR_Version_HasFeature(const char* feature);

	/*! Retrieves the version of this API in the format (major << 24) | (minor << 16) | patch
		@return uint32_t representing version of the API
	*/
	HLVR_RETURN(uint32_t) HLVR_Version_Get(void);
	
	/*! Checks if this header file is compatible (i.e. same major version) with the supplied library.
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
		@see HLVR_System_ResumeEffects
	*/
	HLVR_RETURN(HLVR_Result) HLVR_System_SuspendEffects(HLVR_System* system);

	/*! Resume execution of the effects engine, useful within a game resume routine
		Only effects that were paused due to a call to HLVR_System_SuspendEffects will be resumed
		@param system current context
		@return HLVR_Ok on success
		@see HLVR_System_SuspendEffects
	*/
	HLVR_RETURN(HLVR_Result) HLVR_System_ResumeEffects(HLVR_System* system);

	/*! Cancel all effects, invalidating all HLVR_Effect handles, useful within a game quit routine or a level change 
		where only new HLVR_Effects will be used
		@param system current context
		@return HLVR_Ok on success
	*/
	HLVR_RETURN(HLVR_Result) HLVR_System_CancelEffects(HLVR_System* system);


	/*! Initialize an HLVR_DeviceIterator
	*/
	HLVR_RETURN(HLVR_Result) HLVR_DeviceIterator_Init(HLVR_DeviceIterator* iter);

	/*! Advance the iterator to the next device in the system

		Usage: 
			@code
			HLVR_System* system; //created elsewhere
			HLVR_DeviceIterator deviceIt;
			HLVR_DeviceIterator_Init(&deviceIt);
			while (HLVR_OK(HLVR_DeviceIterator_Next(&deviceIt, system))) {
				std::cout << deviceIt.DeviceInfo.Id << '\n';
			}
			@endcode
		@param iter target iterator
		@param system to the current context
		@return HLVR_Ok if a device was retrieved, HLVR_Error_NoMoreDevices if the iterator is finished.
	*/
	HLVR_RETURN(HLVR_Result) HLVR_DeviceIterator_Next(HLVR_DeviceIterator* iter, HLVR_System* system);


	/*! Initialize an HLVR_NodeIterator
		@param iter target iterator
	*/
	HLVR_RETURN(HLVR_Result) HLVR_NodeIterator_Init(HLVR_NodeIterator* iter);

	/*! Advance the iterator to the next node in the system

		Usage for enumerating all nodes on a specific device:
			@code
			HLVR_System* system; //created elsewhere
			uint32_t my_device = 13;
			HLVR_NodeIterator nodeIt;
			HLVR_NodeIterator_Init(&nodeIt);
			while (HLVR_OK(HLVR_NodeIterator_Next(&nodeIt, my_device, system))) {
				std::cout << nodeIt.NodeInfo.Id << '\n';
			}
			@endcode

		Usage for enumerating all nodes:
			@code
			HLVR_System* system; //created elsewhere
			HLVR_NodeIterator nodeIt;
			HLVR_NodeIterator_Init(&nodeIt);
			while (HLVR_OK(HLVR_NodeIterator_Next(&nodeIt, hlvr_allnodes, system))) {
				std::cout << nodeIt.NodeInfo.Id << '\n';
			}
			@endcode

		@param iter target iterator
		@param device_id the device from which to retrieve retrieve nodes. If special ID hlvr_allnodes is supplied,
		all nodes in the system will be enumerated. If an unknown device id is supplied, the iterator will return HLVR_Error_NoMoreNodes.
		@param system current context
		@return HLVR_Ok if a node was retrieved, HLVR_Error_NoMoreNodes if the iterator is finished.
	*/
	HLVR_RETURN(HLVR_Result) HLVR_NodeIterator_Next(HLVR_NodeIterator* iter, uint32_t device_id, HLVR_System* system);

	
	
	/*! Create a new HLVR_EventData object
		@return HLVR_Ok on success
	*/
	HLVR_RETURN(HLVR_Result) HLVR_EventData_Create(HLVR_EventData** event);
	/*! Destroy an HLVR_EventData object
		@param event object to destroy
	*/
	HLVR_RETURN(void)		 HLVR_EventData_Destroy(HLVR_EventData** event);

	/*! Set a float with the given key 
		@param event target
		@param key key
		@param value value
		@return HLVR_Ok 
	*/
	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetFloat(HLVR_EventData* event, HLVR_EventDataKey key, float value);
	/*! Set an array of floats with the given key
		@param event target
		@param key key
		@param values value array
		@param length length of @p values
		@return HLVR_Ok
	*/
	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetFloats(HLVR_EventData* event, HLVR_EventDataKey key, float* values, unsigned int length);
	/*! Set an int with the given key
		@param event target
		@param key key
		@param value value
		@return HLVR_Ok
	*/
	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetInt(HLVR_EventData* event, HLVR_EventDataKey key, int value);
	/*! Set an array of ints with the given key
		@param event target
		@param key key
		@param values value array
		@param length length of @p values
		@return HLVR_Ok
	*/
	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetInts(HLVR_EventData* event, HLVR_EventDataKey key, int* values, unsigned int length);
	/*! Set an unsigned 32-bit int with the given key
		@param event target
		@param key key
		@param value value
		@return HLVR_Ok
	*/
	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetUInt32(HLVR_EventData* event, HLVR_EventDataKey key, uint32_t value);
	/*! Set an unsigned 32-bit int array with the given key
		@param event target
		@param key key
		@param values value array
		@param length length of @p values
		@return HLVR_Ok
	*/
	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetUInt32s(HLVR_EventData * event, HLVR_EventDataKey key, uint32_t* values, unsigned int length);
	/*! Set an unsigned 64-bit int with the given key
		@param event target
		@param key key
		@param value value
		@return HLVR_Ok
	*/
	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetUInt64(HLVR_EventData * event, HLVR_EventDataKey key, uint64_t value);
	/*! Set an unsigned 64-bit int array with the given key
		@param event target
		@param key key
		@param values value array
		@param length length of @p values
		@return HLVR_Ok
	*/
	HLVR_RETURN(HLVR_Result) HLVR_EventData_SetUInt64s(HLVR_EventData * event, HLVR_EventDataKey key, uint64_t* values, unsigned int length);

	/*! 
		The error value associated with the parse result of a given key
	*/
	typedef enum HLVR_EventData_KeyParseError {
		HLVR_EventData_KeyParseError_UNKNOWN,
		HLVR_EventData_KeyParseError_KeyRequired,
		HLVR_EventData_KeyParseError_InvalidValue,		/*!< the value did not satisfy constraints of the event, e.g. only non-negative numbers */
		HLVR_EventData_KeyParseError_WrongValueType,	/*!< the value exists for the given key, yet it is of the wrong type */
		HLVR_EventData_KeyParseError_MIN = hlvr_int32min,
		HLVR_EventData_KeyParseError_MAX = hlvr_int32max
	} HLVR_EventData_KeyParseError;


	/*! Parse error for a particular key */
	typedef struct HLVR_EventData_KeyParseResult {
		HLVR_EventDataKey Key;
		HLVR_EventData_KeyParseError Error;
	} HLVR_EventData_KeyParseResult;

	/*! Set of results values generated by HLVR_EventData_Validate */
	typedef struct HLVR_EventData_ValidationResult {
		/*! How many errors */
		int Count;
		/*! Value of the errors */
		HLVR_EventData_KeyParseResult Errors[32];
	} HLVR_EventData_ValidationResult;

	/*! Validate an HLVR_EventData object against the parser for the given event type.

		Useful as a debugging tool when writing wrappers for other languages.

		@param event the event to validate
		@param type the type of the event to attempt parsing
		@param outResult where to put validation results 
		@return HLVR_Error_InvalidEventType if the given type is unknown, else HLVR_Ok
	*/
	HLVR_RETURN(HLVR_Result) HLVR_EventData_Validate(HLVR_EventData* event, HLVR_EventType type, HLVR_EventData_ValidationResult* outResult);

	
	/*! Create a new HLVR_Timeline
		@return HLVR_Ok on success
	*/
	HLVR_RETURN(HLVR_Result) HLVR_Timeline_Create(HLVR_Timeline** timeline);
	
	/*! Destroy the given HLVR_Timeline
		@return HLVR_Ok on success
	*/
	HLVR_RETURN(void)		 HLVR_Timeline_Destroy(HLVR_Timeline** timeline);
	
	/*! Add an event with a given type and time offset to this timeline
		@param timeline the timeline to operate on
		@param timeOffsetSeconds the time offset of the event, in fractional seconds (e.g. 3.4f)
		@param data the data to use when adding the event
		@param dataType the type of data to attempt parsing
		@return HLVR_Error_InvalidTimeOffset if @p timeOffsetSeconds < 0, else HLVR_Ok
	*/
	HLVR_RETURN(HLVR_Result) HLVR_Timeline_AddEvent(HLVR_Timeline* timeline, float timeOffsetSeconds, HLVR_EventData* data, HLVR_EventType dataType);
	
	/*! Create a new effect within the system, using a timeline as the source
		@param timeline the source timeline
		@param system current context
		@param effect target HLVR_Effect 
		@return HLVR_Error_EmptyTimeline if timeline contains no events, else HLVR_Ok 
	*/
	HLVR_RETURN(HLVR_Result) HLVR_Timeline_Transmit(HLVR_Timeline* timeline, HLVR_System* system, HLVR_Effect* effect);

	/*! Create a new HLVR_Effect
		@return HLVR_Ok on success
	*/
	HLVR_RETURN(HLVR_Result) HLVR_Effect_Create(HLVR_Effect** handlePtr);
	/*! Destroy the given HLVR_Effect
		The effect will still play out until completion.
		@return HLVR_Ok on success
	*/
	HLVR_RETURN(void)		 HLVR_Effect_Destroy(HLVR_Effect** handlePtr);
	/*! Cause the effect to play, or resume playback from a paused state
		@return HLVR_Ok on success
	*/

	//TODO(casey): return NoSuchHandle if these are called on a newly created but not yet bound handle or if engine is nullptr?
	//Or rework so we are sending in the dependency? Think about that

	HLVR_RETURN(HLVR_Result) HLVR_Effect_Play(HLVR_Effect* handle);
	/*! Cause the effect to pause
		@return HLVR_Ok on success
	*/
	HLVR_RETURN(HLVR_Result) HLVR_Effect_Pause(HLVR_Effect* handle);
	/*! Cause the effect to scrub back to beginning, placing the effect into the idle state
		@return HLVR_Ok on success
	*/
	HLVR_RETURN(HLVR_Result) HLVR_Effect_Reset(HLVR_Effect* handle);
	/*! Return information associated with this effect
		@return HLVR_Error_NoSuchHandle if the given effect is not found, else HLVR_Ok on success
	*/
	HLVR_RETURN(HLVR_Result) HLVR_Effect_GetInfo(HLVR_Effect* effect, HLVR_EffectInfo* info);






#ifdef __cplusplus
}
#endif

