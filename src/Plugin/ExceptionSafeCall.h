#pragma once
#include "HLVR_Errors.h"

#define CATCH_EXCEPTIONS

template<typename T> HLVR_Result ExceptionGuard(T&& t) {
#ifdef CATCH_EXCEPTIONS
	try {
#endif
		return t();
#ifdef CATCH_EXCEPTIONS
	}
	catch (const std::exception& e) {
		BOOST_LOG_TRIVIAL(error) << std::this_thread::get_id() <<
			"[NSLOADER] Exception: " << e.what();
		return HLVR_Error_Unknown;
	}
#endif
}