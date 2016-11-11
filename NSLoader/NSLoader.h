#pragma once

#define NSLOADER_EXPORTS

#ifdef NSLOADER_EXPORTS
#define NSLOADER_API __declspec(dllexport) 
#else
#define NSLOADER_API __declspec(dllimport) 
#endif

#include "TestClass.h"


extern "C" {
	NSLOADER_API TestClass* __stdcall NSVR_Create(LPSTR str) { return new TestClass(str); }
	NSLOADER_API uint32_t __stdcall NSVR_GenHandle(TestClass* ptr) { return ptr->GenHandle(); }
	NSLOADER_API int __stdcall NSVR_PollStatus(TestClass* ptr) { return ptr->PollStatus(); }
	NSLOADER_API void __stdcall NSVR_PollTracking(TestClass* ptr, NullSpaceDLL::TrackingUpdate& q) { ptr->PollTracking(q); }
	NSLOADER_API void __stdcall NSVR_Delete(TestClass* ptr) { delete ptr; }
	NSLOADER_API void __stdcall NSVR_SetTrackingEnabled(TestClass* ptr, bool wantTracking) { ptr->SetTrackingEnabled(wantTracking); }
	
	//Sequence
	NSLOADER_API void __stdcall NSVR_CreateSequence(TestClass* ptr, uint32_t handle, LPSTR name, uint32_t loc) {
		ptr->CreateSequence(handle, name, loc);
	}
	NSLOADER_API bool __stdcall NSVR_LoadSequence(TestClass* ptr, LPSTR param) { return ptr->LoadSequence(param); }
	
	//Pattern
	NSLOADER_API void __stdcall NSVR_CreatePattern(TestClass* ptr, uint32_t handle, LPSTR name) {
		ptr->CreatePattern(handle, name);
	}
	NSLOADER_API bool __stdcall NSVR_LoadPattern(TestClass* ptr, LPSTR param) { return ptr->LoadPattern(param); }


	NSLOADER_API void __stdcall NSVR_HandleCommand(TestClass* ptr, unsigned int handle, short command) { ptr->HandleCommand(handle, command); }
}