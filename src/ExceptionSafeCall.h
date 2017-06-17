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
	catch (const std::exception& e) {
		BOOST_LOG_TRIVIAL(error) << std::this_thread::get_id() <<
			"[NSLOADER] Exception: " << e.what();
		return NSVR_Error_Unknown;
	}
#endif
}