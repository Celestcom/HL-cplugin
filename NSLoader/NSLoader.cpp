// NSLoader.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NSLoader.h"
#include "Engine.h"

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

NSLOADER_API NSVR_Context_t* __stdcall NSVR_Create()
{
	return AS_TYPE(NSVR_Context_t, new Engine());
}

 NSLOADER_API unsigned int __stdcall NSVR_GenHandle(NSVR_Context_t* ptr)
 {
	 return AS_TYPE(Engine, ptr)->GenHandle();

 }

 NSLOADER_API int __stdcall NSVR_PollStatus(NSVR_Context_t* ptr)
 {
	return AS_TYPE(Engine, ptr)->PollStatus();
 }

 NSLOADER_API void __stdcall NSVR_PollTracking(NSVR_Context_t* ptr, NSVR_InteropTrackingUpdate & q)
 {
	 return AS_TYPE(Engine, ptr)->PollTracking(q);
 }

 NSLOADER_API void __stdcall NSVR_Delete(NSVR_Context_t* ptr)
 {
	 if (!ptr) {
		 return;
	 }
	 delete AS_TYPE(Engine,ptr);
 }






 NSLOADER_API void __stdcall NSVR_DoHandleCommand(NSVR_Context_t* ptr, uint32_t handle, NSVR_HandleCommand command)
 {
	 return AS_TYPE(Engine, ptr)->HandleCommand(handle, command);
 }

 NSLOADER_API char * __stdcall NSVR_GetError(NSVR_Context_t* ptr)
 {
	 return AS_TYPE(Engine, ptr)->GetError();
 }

 NSLOADER_API void __stdcall NSVR_FreeError(char * string)
 {
	 delete[] string;
	 string = nullptr;
 }

 NSLOADER_API int __stdcall NSVR_DoEngineCommand(NSVR_Context_t* ptr, NSVR_EngineCommand command)
 {
	  return AS_TYPE(Engine, ptr)->EngineCommand(command);
 }

 

 NSLOADER_API int __stdcall NSVR_TransmitEvents(NSVR_Context_t* ptr, uint32_t handle, void * data, uint32_t size)
 {

	


	 return AS_TYPE(Engine, ptr)->CreateEffect(handle, data, size);
 }

