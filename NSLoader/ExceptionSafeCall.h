#pragma once
#include "NSLoader_Errors.h"

template<typename T> NSVR_Result ExceptionGuard(T&& t) {
	try {
		t();
		return NSVR_Success_Unqualified;
	}
	catch (const std::exception& e) {
		return -1;
	}
}