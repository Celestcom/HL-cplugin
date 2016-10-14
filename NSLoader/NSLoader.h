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
	NSLOADER_API int __stdcall TestClass_Play(TestClass* ptr, LPSTR param) { return ptr->Play(param); }
	NSLOADER_API void __stdcall TestClass_Delete(TestClass* ptr) { delete ptr; }
}