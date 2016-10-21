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
	NSLOADER_API int __stdcall TestClass_PlaySequence(TestClass* ptr, LPSTR param, Location loc) { return ptr->PlaySequence(param, loc); }
	NSLOADER_API int __stdcall TestClass_PlayPattern(TestClass* ptr, LPSTR param, Side side) { return ptr->PlayPattern(param, side); }
	NSLOADER_API void __stdcall TestClass_Delete(TestClass* ptr) { delete ptr; }
}