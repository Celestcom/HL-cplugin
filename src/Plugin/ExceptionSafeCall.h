#pragma once
#include "HLVR_Errors.h"

#define SWALLOW_EXCEPTIONS


template<typename T> HLVR_Result ExceptionGuard(T&& t) {
#ifdef SWALLOW_EXCEPTIONS
	try {
#endif

		return t();

#ifdef SWALLOW_EXCEPTIONS
	}
	catch (const std::bad_alloc&) {
		return HLVR_Error_BadAlloc;
	}
	catch (const std::exception&) {
		return HLVR_Error_Unspecified;
	}
#endif
}