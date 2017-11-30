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
	catch (const std::exception& e) {
		BOOST_LOG_TRIVIAL(error) << std::this_thread::get_id() <<
			"[NSLOADER] Exception: " << e.what();
		return HLVR_Error_Unspecified;
	}
#endif
}