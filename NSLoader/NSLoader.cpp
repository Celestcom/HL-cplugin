// NSLoader.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NSLoader.h"
#include "Engine.h"
#include "EventList.h"
#include "Event.h"
#include "ParameterizedEvent.h"
#include "PlaybackHandle.h"
#include "ExceptionSafeCall.h"

#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)


//comment this line if you want to disable argument null checking. Profile really hard before doing this.
#define NULL_ARGUMENT_CHECKS



#ifdef NULL_ARGUMENT_CHECKS
#define RETURN_IF_NULL(ptr) do { if (ptr == nullptr) { return (NSVR_Result) NSVR_Error_NullArgument; }} while (0)
#else
#define RETURN_IF_NULL(ptr)
#endif


NSLOADER_API unsigned int __stdcall NSVR_GetVersion(void)
{
	return NSLOADER_API_VERSION;
}

NSLOADER_API int _stdcall NSVR_IsCompatibleDLL(void)
{
	unsigned int major = NSVR_GetVersion() >> 16;
	return major == NSLOADER_API_VERSION_MAJOR;
}

NSLOADER_API NSVR_Result __stdcall NSVR_System_GetServiceInfo(NSVR_System * systemPtr, NSVR_ServiceInfo * infoPtr)
{
	RETURN_IF_NULL(systemPtr);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, systemPtr)->PollStatus(infoPtr);
	});

}


NSLOADER_API NSVR_Result __stdcall NSVR_System_GetDeviceInfo(NSVR_System * systemPtr, NSVR_DeviceInfo * infoPtr)
{
	RETURN_IF_NULL(systemPtr);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, systemPtr)->PollDevice(infoPtr);
	});
}

NSLOADER_API NSVR_Result __stdcall NSVR_System_Create(NSVR_System** systemPtr)
{
	return ExceptionGuard([&] { *systemPtr = AS_TYPE(NSVR_System, new Engine()); return NSVR_Success_Unqualified; });
}

NSLOADER_API void __stdcall NSVR_System_Release(NSVR_System** ptr)
{	
	delete AS_TYPE(Engine, *ptr);
	*ptr = nullptr;
}


 NSLOADER_API NSVR_Result __stdcall NSVR_System_Haptics_Pause(NSVR_System* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->EngineCommand(NSVR_EngineCommand_PauseAll);
	 });
 }

 NSLOADER_API NSVR_Result __stdcall NSVR_System_Haptics_Resume(NSVR_System* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->EngineCommand(NSVR_EngineCommand_ResumeAll);
	 });
 }

 NSLOADER_API NSVR_Result __stdcall NSVR_System_Haptics_Destroy(NSVR_System* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->EngineCommand(NSVR_EngineCommand_DestroyAll);
	 });
 }



 NSLOADER_API NSVR_Result __stdcall NSVR_System_Tracking_Poll(NSVR_System * ptr, NSVR_TrackingUpdate * updatePtr)
 {
	 RETURN_IF_NULL(ptr);
	 RETURN_IF_NULL(updatePtr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->PollTracking(updatePtr);

	 });
 }

 NSLOADER_API NSVR_Result __stdcall NSVR_System_Tracking_Enable(NSVR_System * ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->EngineCommand(NSVR_EngineCommand_EnableTracking);
	 });
 }

 NSLOADER_API NSVR_Result __stdcall NSVR_System_Tracking_Disable(NSVR_System * ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->EngineCommand(NSVR_EngineCommand_DisableTracking);
	 });
 }



 NSLOADER_API NSVR_Result  __stdcall NSVR_System_GetError(NSVR_System* ptr, NSVR_ErrorInfo* errorInfo)
 {
	 RETURN_IF_NULL(ptr);
	 RETURN_IF_NULL(errorInfo);

	 return ExceptionGuard([&] {
		 AS_TYPE(Engine, ptr)->GetError(errorInfo);
		 return NSVR_Success_Unqualified;
	 });
 }


 NSLOADER_API NSVR_Result __stdcall NSVR_Event_Create(NSVR_Event** eventPtr, NSVR_EventType type)
 {
	
	 return ExceptionGuard([&] {
		 switch (type) {
		 case NSVR_EventType::NSVR_EventType_BasicHapticEvent:
			 *eventPtr = AS_TYPE(NSVR_Event, new BasicHapticEvent());
		 default:
			 return (NSVR_Result) NSVR_Error_InvalidEventType;
		 }

		 return (NSVR_Result) NSVR_Success_Unqualified;
	 });
 }

 NSLOADER_API void __stdcall NSVR_Event_Release(NSVR_Event ** eventPtr)
 {
	 delete AS_TYPE(ParameterizedEvent, *eventPtr);
	 *eventPtr = nullptr;
 }

 NSLOADER_API NSVR_Result __stdcall NSVR_Event_SetFloat(NSVR_Event * event, const char * key, float value)
 {
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		 return AS_TYPE(ParameterizedEvent, event)->SetFloat(key, value);
	 });
 }

 NSLOADER_API NSVR_Result __stdcall NSVR_Event_SetInteger(NSVR_Event * event, const char * key, int value)
 {
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		 return AS_TYPE(ParameterizedEvent, event)->SetInt(key, value);
	 });
 }

 NSLOADER_API NSVR_Result  __stdcall NSVR_Timeline_Create(NSVR_Timeline** timelinePtr, NSVR_System* systemPtr)
 {
	 RETURN_IF_NULL(systemPtr);

	 return ExceptionGuard([&] {
		 *timelinePtr = AS_TYPE(NSVR_Timeline, new EventList(AS_TYPE(Engine, systemPtr)));
		 return NSVR_Success_Unqualified;
	 });
 }

 

 NSLOADER_API void __stdcall NSVR_Timeline_Release(NSVR_Timeline ** listPtr)
 {
	delete AS_TYPE(EventList, *listPtr);
	*listPtr = nullptr;
 }

 NSLOADER_API NSVR_Result __stdcall NSVR_Timeline_AddEvent(NSVR_Timeline * list, NSVR_Event * event)
 {
	 RETURN_IF_NULL(list);
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		return AS_TYPE(EventList, list)->AddEvent(AS_TYPE(ParameterizedEvent, event));
	 });
 }

 NSLOADER_API NSVR_Result __stdcall NSVR_Timeline_Transmit(NSVR_Timeline * timelinePtr, NSVR_PlaybackHandle * handlePtr)
 {
	 RETURN_IF_NULL(timelinePtr);
	 RETURN_IF_NULL(handlePtr);

	 return ExceptionGuard([&] {

		 return AS_TYPE(PlaybackHandle, handlePtr)->Bind(AS_TYPE(EventList, timelinePtr));
		 
	 });
 }


 NSLOADER_API NSVR_Result  __stdcall NSVR_PlaybackHandle_Create(NSVR_PlaybackHandle ** handlePtr)
 {

	 return ExceptionGuard([&] {
		 *handlePtr = AS_TYPE(NSVR_PlaybackHandle, new PlaybackHandle());
		 return NSVR_Success_Unqualified;
	 });
}

 NSLOADER_API NSVR_Result  __stdcall NSVR_PlaybackHandle_Bind(NSVR_PlaybackHandle * handlePtr, NSVR_Timeline * timelinePtr)
 {
	 RETURN_IF_NULL(handlePtr);
	 RETURN_IF_NULL(timelinePtr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(PlaybackHandle, handlePtr)->Bind(AS_TYPE(EventList, timelinePtr));
	 });
 }

 NSLOADER_API NSVR_Result __stdcall NSVR_PlaybackHandle_Command(NSVR_PlaybackHandle * handlePtr, NSVR_PlaybackCommand command)
 {
	 RETURN_IF_NULL(handlePtr);

	 return ExceptionGuard([&] {
		return AS_TYPE(PlaybackHandle, handlePtr)->Command(command);
	 });
 }

 NSLOADER_API void __stdcall NSVR_PlaybackHandle_Release(NSVR_PlaybackHandle** handlePtr)
 {
	delete AS_TYPE(PlaybackHandle, *handlePtr);
	 *handlePtr = nullptr; 
 }





