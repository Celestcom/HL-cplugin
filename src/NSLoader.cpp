// NSLoader.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NSLoader.h"
#include "Engine.h"
#include "EventList.h"
#include "ParameterizedEvent.h"
#include "PlaybackHandle.h"
#include "ExceptionSafeCall.h"
#include "EngineCommand.h"

#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)


//comment this line if you want to disable argument null checking. Profile really hard before doing this.
#define NULL_ARGUMENT_CHECKS




#ifdef NULL_ARGUMENT_CHECKS
#define RETURN_IF_NULL(ptr) do { if (ptr == nullptr) { return (NSVR_Result) NSVR_Error_NullArgument; }} while (0)
#else
#define RETURN_IF_NULL(ptr)
#endif



NSVR_RETURN(unsigned int) NSVR_Version_Get(void)
{
	return NSLOADER_API_VERSION;
}

NSVR_RETURN(int) NSVR_Version_IsCompatibleDLL(void)
{
	unsigned int major = NSVR_Version_Get() >> 16;
	return major == NSLOADER_API_VERSION_MAJOR;
}

NSVR_RETURN(NSVR_Result) NSVR_System_GetServiceInfo(NSVR_System * systemPtr, NSVR_ServiceInfo * infoPtr)
{
	RETURN_IF_NULL(systemPtr);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, systemPtr)->PollStatus(infoPtr);
	});

}


NSVR_RETURN(NSVR_Result) NSVR_System_GetDeviceInfo(NSVR_System * systemPtr, NSVR_DeviceInfo * infoPtr)
{
	RETURN_IF_NULL(systemPtr);

	return ExceptionGuard([&] {
		return AS_TYPE(Engine, systemPtr)->PollDevice(infoPtr);
	});
}

NSVR_RETURN(NSVR_Result) NSVR_System_Create(NSVR_System** systemPtr)
{

	return ExceptionGuard([&] { *systemPtr = AS_TYPE(NSVR_System, new Engine()); return NSVR_Success_Unqualified; });
}

NSVR_RETURN(int) NSVR_Version_HasFeature(const char * feature)
{
	static std::set<std::string> features;

	if (features.empty())
	{
		//cache the feature list
		//features.insert("TRACKING");
		//features.insert("STREAMING");
		//whatever
	}

	return features.find(feature) != features.end();
}

NSVR_RETURN(void) NSVR_System_Release(NSVR_System** ptr)
{	
	ExceptionGuard([&] {
		delete AS_TYPE(Engine, *ptr);
		*ptr = nullptr;

		return NSVR_Success_Unqualified;
	});
}


NSVR_RETURN(NSVR_Result) NSVR_System_Haptics_Pause(NSVR_System* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::PauseAll);
	 });
 }

NSVR_RETURN(NSVR_Result) NSVR_System_Haptics_Resume(NSVR_System* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::ResumeAll);
	 });
 }

NSVR_RETURN(NSVR_Result) NSVR_System_Haptics_Destroy(NSVR_System* ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::DestroyAll);
	 });
 }



NSVR_RETURN(NSVR_Result) NSVR_System_Tracking_Poll(NSVR_System * ptr, NSVR_TrackingUpdate * updatePtr)
 {
	 RETURN_IF_NULL(ptr);
	 RETURN_IF_NULL(updatePtr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->PollTracking(updatePtr);

	 });
 }

NSVR_RETURN(NSVR_Result) NSVR_System_Tracking_Enable(NSVR_System * ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::EnableTracking);
	 });
 }

NSVR_RETURN(NSVR_Result) NSVR_System_Tracking_Disable(NSVR_System * ptr)
 {
	 RETURN_IF_NULL(ptr);

	 return ExceptionGuard([&] {
		 return AS_TYPE(Engine, ptr)->DoEngineCommand(EngineCommand::DisableTracking);
	 });
 }



NSVR_RETURN(NSVR_Result) NSVR_Event_Create(NSVR_Event** eventPtr, NSVR_EventType type)
 {
	
	return ExceptionGuard([&] {
	
		ParameterizedEvent* newEvent = new ParameterizedEvent(type);
		if (newEvent == nullptr) { 
			return (NSVR_Result)NSVR_Error_InvalidEventType; 
		}

		*eventPtr = AS_TYPE(NSVR_Event, newEvent);
		 return (NSVR_Result) NSVR_Success_Unqualified;
	 });
 }

NSVR_RETURN(void) NSVR_Event_Release(NSVR_Event ** eventPtr)
 {
	ExceptionGuard([&] {
		delete AS_TYPE(ParameterizedEvent, *eventPtr);
		*eventPtr = nullptr;

		return NSVR_Success_Unqualified;
	});
 }

NSVR_RETURN(NSVR_Result) NSVR_Event_SetFloat(NSVR_Event * event, const char * key, float value)
 {
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		 return AS_TYPE(ParameterizedEvent, event)->SetFloat(key, value);
	 });
 }

NSVR_RETURN(NSVR_Result)NSVR_Event_SetInteger(NSVR_Event * event, const char * key, int value)
 {
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		 return AS_TYPE(ParameterizedEvent, event)->SetInt(key, value);
	 });
 }

NSVR_RETURN(NSVR_Result)NSVR_Timeline_Create(NSVR_Timeline** timelinePtr)
 {

	 return ExceptionGuard([&] {
		 *timelinePtr = AS_TYPE(NSVR_Timeline, new EventList());
		 return NSVR_Success_Unqualified;
	 });
 }

 

NSVR_RETURN(void) NSVR_Timeline_Release(NSVR_Timeline ** listPtr)
 {
	ExceptionGuard([&] {
		delete AS_TYPE(EventList, *listPtr);
		*listPtr = nullptr;

		return NSVR_Success_Unqualified;
	});
 }

NSVR_RETURN(NSVR_Result) NSVR_Timeline_AddEvent(NSVR_Timeline * list, NSVR_Event * event)
 {
	 RETURN_IF_NULL(list);
	 RETURN_IF_NULL(event);

	 return ExceptionGuard([&] {
		return AS_TYPE(EventList, list)->AddEvent(AS_TYPE(ParameterizedEvent, event));
	 });
 }

NSVR_RETURN(NSVR_Result) NSVR_Timeline_Transmit(NSVR_Timeline * timelinePtr, NSVR_System* systemPtr, NSVR_PlaybackHandle * handlePtr)
 {
	 RETURN_IF_NULL(systemPtr);
	 RETURN_IF_NULL(timelinePtr);
	 RETURN_IF_NULL(handlePtr);

	 return ExceptionGuard([&] {
		 auto engine = AS_TYPE(Engine, systemPtr);
		 auto timeline = AS_TYPE(EventList, timelinePtr);
		 auto handle = AS_TYPE(PlaybackHandle, handlePtr);

		
		 if (handle->handle != 0) {
			 engine->ReleaseHandle(handle->handle);
		 }
		
		 handle->engine = engine;
		 return engine->CreateEffect(timeline, &handle->handle);
		 
	 });
 }


NSVR_RETURN(NSVR_Result) NSVR_PlaybackHandle_Create(NSVR_PlaybackHandle ** handlePtr)
 {

	 return ExceptionGuard([&] {
		 *handlePtr = AS_TYPE(NSVR_PlaybackHandle, new PlaybackHandle());
		 return NSVR_Success_Unqualified;
	 });

	 
}



NSVR_RETURN(NSVR_Result)NSVR_PlaybackHandle_Command(NSVR_PlaybackHandle * handlePtr, NSVR_PlaybackCommand command)
 {
	 RETURN_IF_NULL(handlePtr);

	 return ExceptionGuard([&] {
		return AS_TYPE(PlaybackHandle, handlePtr)->Command(command);
	 });
 }

NSVR_RETURN(void) NSVR_PlaybackHandle_Release(NSVR_PlaybackHandle** handlePtr)
 {
	ExceptionGuard([&] {

		delete AS_TYPE(PlaybackHandle, *handlePtr);
		*handlePtr = nullptr;

		return NSVR_Success_Unqualified;
	});
 }



NSVR_RETURN(NSVR_Result) NSVR_PlaybackHandle_GetInfo(NSVR_PlaybackHandle* handlePtr, NSVR_HandleInfo* infoPtr)
{
	RETURN_IF_NULL(handlePtr);
	RETURN_IF_NULL(infoPtr);

	return ExceptionGuard([&] {
		return AS_TYPE(PlaybackHandle, handlePtr)->GetHandleInfo(infoPtr);
	});
}




