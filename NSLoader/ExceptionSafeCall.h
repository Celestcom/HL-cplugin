#pragma once
#include "NSLoader_Errors.h"

template<typename T> NSVR_Result ExceptionGuard(T&& t) {
	try {
		return t();
	}
	catch (const std::exception&) {
		return NSVR_Error_Unknown;
	}
}