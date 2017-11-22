#pragma once

#include "HLVR.h"
#include <memory>
namespace hlvr 
{
namespace detail {


template<typename T, typename Creator, typename Deleter>
class native_handle_owner {
public:
	explicit native_handle_owner(Creator c, Deleter d)
		: m_handle{ nullptr, d }
		, m_lastError{ 0 }
	{
		T* tempHandle = nullptr;
		m_lastError = c(&tempHandle);

		if (HLVR_OK(m_lastError)) {
			m_handle.reset(tempHandle);
		}
	}

	const T* native_handle() const {
		return m_handle.get();
	}

	T* native_handle() {
		return m_handle.get();
	}

	explicit operator bool() const noexcept {
		return (bool)m_handle;
	}

	HLVR_Result last_error() const noexcept {
		return m_lastError;
	}
protected:
	std::unique_ptr<T, Deleter> m_handle;
	HLVR_Result m_lastError;
};

}
}