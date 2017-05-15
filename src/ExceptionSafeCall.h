#pragma once
#include "NSLoader_Errors.h"

#define CATCH_EXCEPTIONS

template<typename T> NSVR_Result ExceptionGuard(T&& t) {
#ifdef CATCH_EXCEPTIONS
	try {
#endif
		return t();
#ifdef CATCH_EXCEPTIONS
	}
	catch (const std::exception&) {
		return NSVR_Error_Unknown;
	}
#endif
}