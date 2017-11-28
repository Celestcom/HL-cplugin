#pragma once

#include "HLVR.h"
#include <memory>
#include <cassert>
namespace hlvr {
namespace detail {

template<typename T, typename Creator, typename Deleter>
class native_handle_owner {
public:
	explicit native_handle_owner(Creator c, Deleter d)
		: m_handle{ nullptr, d }
	{
		T* tempHandle = nullptr;
		HLVR_Result ec = c(&tempHandle);

		if (HLVR_OK(ec)) {
			m_handle.reset(tempHandle);
		}
	}

	const T* native_handle() const noexcept{
		return m_handle.get();
	}

	T* native_handle() noexcept {
		return m_handle.get();
	}

	explicit operator bool() const noexcept {
		return (bool) m_handle;
	}


protected:
	std::unique_ptr<T, Deleter> m_handle;
};

}
}