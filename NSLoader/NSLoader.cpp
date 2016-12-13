// NSLoader.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NSLoader.h"
#include "TestClass.h"

 NSVRPlugin __stdcall NSVR_Create()
{
	return reinterpret_cast<NSVRPlugin>(new TestClass());
}

 NSLOADER_API unsigned int __stdcall NSVR_GenHandle(NSVRPlugin ptr)
 {
	 return reinterpret_cast<TestClass*>(ptr)->GenHandle();

 }

 NSLOADER_API int __stdcall NSVR_PollStatus(NSVRPlugin ptr)
 {
	 return reinterpret_cast<TestClass*>(ptr)->PollStatus();
 }

 NSLOADER_API void __stdcall NSVR_PollTracking(NSVRPlugin ptr, NullSpaceDLL::InteropTrackingUpdate & q)
 {
	 return reinterpret_cast<TestClass*>(ptr)->PollTracking(q);
 }

 NSLOADER_API void __stdcall NSVR_Delete(NSVRPlugin ptr)
 {
	 delete reinterpret_cast<TestClass*>(ptr);
 }

 NSLOADER_API bool __stdcall NSVR_CreateSequence(NSVRPlugin ptr, unsigned int handle, LPSTR name, unsigned int loc)
 {
	 return reinterpret_cast<TestClass*>(ptr)->CreateSequence(handle, name, loc);
 }

 NSLOADER_API bool __stdcall NSVR_Load(NSVRPlugin ptr, LPSTR param, int fileType)
 {
	 return reinterpret_cast<TestClass*>(ptr)->Load(param, fileType);
 }

 NSLOADER_API bool __stdcall NSVR_CreatePattern(NSVRPlugin ptr, unsigned int handle, LPSTR name)
 {
	 return reinterpret_cast<TestClass*>(ptr)->CreatePattern(handle, name);
 }

 NSLOADER_API bool __stdcall NSVR_CreateExperience(NSVRPlugin ptr, unsigned int handle, LPSTR name)
 {
	 return reinterpret_cast<TestClass*>(ptr)->CreateExperience(handle, name);
 }

 NSLOADER_API void __stdcall NSVR_HandleCommand(NSVRPlugin ptr, unsigned int handle, short command)
 {
	 return reinterpret_cast<TestClass*>(ptr)->HandleCommand(handle, command);
 }

 NSLOADER_API char * __stdcall NSVR_GetError(NSVRPlugin ptr)
 {
	 return reinterpret_cast<TestClass*>(ptr)->GetError();
 }

 NSLOADER_API void __stdcall NSVR_FreeString(char * string)
 {
	 delete[] string;
 }

 NSLOADER_API bool __stdcall NSVR_EngineCommand(NSVRPlugin ptr, short command)
 {
	 return reinterpret_cast<TestClass*>(ptr)->EngineCommand(command);
 }

 NSLOADER_API bool __stdcall NSVR_InitializeFromFilesystem(NSVRPlugin ptr, LPSTR path)
 {
	 return reinterpret_cast<TestClass*>(ptr)->InitializeFromFilesystem(path);
 }

