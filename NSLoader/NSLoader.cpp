// NSLoader.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NSLoader.h"
#include "Engine.h"

 NSVRPlugin __stdcall NSVR_Create()
{
	return reinterpret_cast<NSVRPlugin>(new Engine());
}

 NSLOADER_API unsigned int __stdcall NSVR_GenHandle(NSVRPlugin ptr)
 {
	 return reinterpret_cast<Engine*>(ptr)->GenHandle();

 }

 NSLOADER_API int __stdcall NSVR_PollStatus(NSVRPlugin ptr)
 {
	 return reinterpret_cast<Engine*>(ptr)->PollStatus();
 }

 NSLOADER_API void __stdcall NSVR_PollTracking(NSVRPlugin ptr, NullSpaceDLL::InteropTrackingUpdate & q)
 {
	// return reinterpret_cast<TestClass*>(ptr)->PollTracking(q);
 }

 NSLOADER_API void __stdcall NSVR_Delete(NSVRPlugin ptr)
 {
	 delete reinterpret_cast<Engine*>(ptr);
 }



 NSLOADER_API bool __stdcall NSVR_Load(NSVRPlugin ptr, LPSTR param, int fileType)
 {
//	 return reinterpret_cast<TestClass*>(ptr)->Load(param, fileType);
	return true;
 }




 NSLOADER_API void __stdcall NSVR_HandleCommand(NSVRPlugin ptr, unsigned int handle, short command)
 {
	 return reinterpret_cast<Engine*>(ptr)->HandleCommand(handle, command);
 }

 NSLOADER_API char * __stdcall NSVR_GetError(NSVRPlugin ptr)
 {
	 return reinterpret_cast<Engine*>(ptr)->GetError();
 }

 NSLOADER_API void __stdcall NSVR_FreeString(char * string)
 {
	 delete[] string;
	 string = nullptr;
 }

 NSLOADER_API bool __stdcall NSVR_EngineCommand(NSVRPlugin ptr, short command)
 {
	 return reinterpret_cast<Engine*>(ptr)->EngineCommand(command);
 }

 NSLOADER_API bool __stdcall NSVR_InitializeFromFilesystem(NSVRPlugin ptr, LPSTR path)
 {
	 return reinterpret_cast<Engine*>(ptr)->InitializeFromFilesystem(path);
 }

 NSLOADER_API void __stdcall NSVR_CreateHaptic(NSVRPlugin ptr, unsigned int handle, void * data, unsigned int size)
 {
	 reinterpret_cast<Engine*>(ptr)->CreateHaptic(handle, data, size);
 }

