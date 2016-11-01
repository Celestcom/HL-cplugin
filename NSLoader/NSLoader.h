#pragma once

#define NSLOADER_EXPORTS

#ifdef NSLOADER_EXPORTS
#define NSLOADER_API __declspec(dllexport) 
#else
#define NSLOADER_API __declspec(dllimport) 
#endif

#include "TestClass.h"


extern "C" {
	NSLOADER_API TestClass* __stdcall TestClass_Create(LPSTR str) { return new TestClass(str); }
	NSLOADER_API int __stdcall TestClass_PlayEffect(TestClass* ptr, Effect effect, Location loc, float duration, float time, unsigned int priority) 
	{ return ptr->PlayEffect(effect, loc, duration, time, priority); }
	NSLOADER_API int __stdcall TestClass_PlaySequence(TestClass* ptr, LPSTR param, Location loc) { return ptr->PlaySequence(param, loc); }
	NSLOADER_API int __stdcall TestClass_PlayPattern(TestClass* ptr, LPSTR param, Side side) { return ptr->PlayPattern(param, side); }
	NSLOADER_API int __stdcall TestClass_PollStatus(TestClass* ptr) { return ptr->PollStatus(); }
	NSLOADER_API void __stdcall TestClass_PollTracking(TestClass* ptr, NullSpaceDLL::TrackingUpdate& q) { ptr->PollTracking(q); }
	NSLOADER_API void __stdcall TestClass_Delete(TestClass* ptr) { delete ptr; }
}