// NSLoader.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NSLoader.h"
#include "Engine.h"
#include "EventList.h"
#include "Event.h"
#include "ParameterizedEvent.h"
#define AS_TYPE(Type, Obj) reinterpret_cast<Type *>(Obj)
#define AS_CTYPE(Type, Obj) reinterpret_cast<const Type *>(Obj)

NSLOADER_API unsigned int __stdcall NSVR_GetVersion(void)
{
	return NSLOADER_API_VERSION;
}

NSLOADER_API int _stdcall NSVR_IsCompatibleDLL(void)
{
	unsigned int major = NSVR_GetVersion() >> 16;
	return major == NSLOADER_API_VERSION_MAJOR;
}



NSLOADER_API NSVR_System* __stdcall NSVR_System_Create()
{
	return AS_TYPE(NSVR_System, new Engine());
}


 NSLOADER_API unsigned int __stdcall NSVR_System_GenerateHandle(NSVR_System* ptr)
 {
	 return AS_TYPE(Engine, ptr)->GenHandle();

 }

 NSLOADER_API NSVR_Result __stdcall NSVR_System_PollStatus(NSVR_System* ptr, NSVR_System_Status* status)
 {
	return AS_TYPE(Engine, ptr)->PollStatus(status);
 }

 NSLOADER_API void __stdcall NSVR_System_PollTracking(NSVR_System* ptr, NSVR_TrackingUpdate & q)
 {
	 return AS_TYPE(Engine, ptr)->PollTracking(q);
 }

 NSLOADER_API void __stdcall NSVR_System_Release(NSVR_System* ptr)
 {
	 if (!ptr) {
		 return;
	 }
	 delete AS_TYPE(Engine,ptr);
 }






 NSLOADER_API void __stdcall NSVR_System_DoHandleCommand(NSVR_System* ptr, uint32_t handle, NSVR_HandleCommand command)
 {
	 return AS_TYPE(Engine, ptr)->HandleCommand(handle, command);
 }

 NSLOADER_API char * __stdcall NSVR_System_GetError(NSVR_System* ptr)
 {
	 return AS_TYPE(Engine, ptr)->GetError();
 }

 NSLOADER_API void __stdcall NSVR_FreeError(char * string)
 {
	 delete[] string;
	 string = nullptr;
 }

 NSLOADER_API NSVR_Event * __stdcall NSVR_Event_Create(NSVR_EventType type)
 {
	 switch (type) {
	 case NSVR_EventType::BASIC_HAPTIC_EVENT:
		 return AS_TYPE(NSVR_Event, new BasicHapticEvent());
	 default:
		 return nullptr;
	 }
 }

 NSLOADER_API void __stdcall NSVR_Event_Release(NSVR_Event * event)
 {
	 if (event == nullptr) {
		 return;
	 }

	 delete AS_TYPE(ParameterizedEvent, event);
	 event = nullptr;
 }

 NSLOADER_API NSVR_Result __stdcall NSVR_Event_SetFloat(NSVR_Event * event, const char * key, float value)
 {
	 return AS_TYPE(ParameterizedEvent, event)->SetFloat(key, value);
 }

 NSLOADER_API NSVR_Result __stdcall NSVR_Event_SetInteger(NSVR_Event * event, const char * key, int value)
 {
	return AS_TYPE(ParameterizedEvent, event)->SetInt(key, value);
 }

 NSLOADER_API NSVR_EventList* __stdcall NSVR_EventList_Create()
 {
	 return AS_TYPE(NSVR_EventList, new EventList());
 }

 

 NSLOADER_API void __stdcall NSVR_EventList_Release(NSVR_EventList * listPtr)
 {
	 if (listPtr == nullptr) {
		 return;
	 }
	 delete AS_TYPE(EventList, listPtr);
	 listPtr = nullptr;
 }

 NSLOADER_API NSVR_Result __stdcall NSVR_EventList_AddEvent(NSVR_EventList * list, NSVR_Event * event)
 {
	 return AS_TYPE(EventList, list)->AddEvent(AS_TYPE(ParameterizedEvent, event));
 }

 NSLOADER_API NSVR_Result __stdcall NSVR_EventList_Bind(NSVR_System * ptr, NSVR_EventList* listPtr, uint32_t handle)
 {
	 AS_TYPE(Engine, ptr)->CreateEffect(AS_TYPE(EventList,listPtr), handle);
	 return 1;
 }

 NSLOADER_API int __stdcall NSVR_System_DoEngineCommand(NSVR_System* ptr, NSVR_EngineCommand command)
 {
	  return AS_TYPE(Engine, ptr)->EngineCommand(command);
 }

 

 NSLOADER_API int __stdcall NSVR_System_TransmitEvents(NSVR_System* ptr, uint32_t handle, void * data, uint32_t size)
 {

	


	 return AS_TYPE(Engine, ptr)->CreateEffect(handle, data, size);
 }

