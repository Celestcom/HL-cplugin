#pragma once

#include "HLVR.h"
#include <memory>
#include <cassert>
#include "expected.hpp"
#include "../hlvr_error.hpp"

namespace hlvr {
	namespace detail {
		template<typename NativeHandle, typename Creator, typename Deleter>
		struct native_traits {
			using native_handle_type = NativeHandle;
			using creator_type = Creator;
			using deleter_type = Deleter;
		};
		/*template<typename Wrapped, typename Traits>
		static tl::expected<Wrapped, status_code> make_helper(typename Traits::creator_type c) {
			Traits::native_handle_type* handle = nullptr;
			HLVR_Result sc = c(&handle);
			if (HLVR_OK(sc)) {
				return Wrapped(handle);
			}
			else {
				return tl::make_unexpected(status_code(sc));
			}
		}*/

		template<typename T, class Traits>
		class native_handle_owner {
	
		public:


			typename  const Traits::native_handle_type* native_handle() const noexcept {
				return m_handle.get();
			}

			typename Traits::native_handle_type* native_handle() noexcept {
				return m_handle.get();
			}

			explicit operator bool() const noexcept {
				return (bool)m_handle;
			}
			native_handle_owner(typename Traits::deleter_type d) : m_handle{nullptr, d} {}

			

		protected:
			static tl::expected<T, status_code> make_helper(typename Traits::creator_type c) {
				Traits::native_handle_type* handle = nullptr;
				HLVR_Result sc = c(&handle);
				if (HLVR_OK(sc)) {
					return T(handle);
				}
				else {
					return tl::make_unexpected(status_code(sc));
				}
			}
			explicit native_handle_owner(typename Traits::native_handle_type* raw_ptr, typename Traits::deleter_type del) : m_handle{ nullptr, del } {
				m_handle.reset(raw_ptr);
			}

			std::unique_ptr<typename Traits::native_handle_type, typename Traits::deleter_type> m_handle;


		};

	}
}