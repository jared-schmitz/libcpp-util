//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_REF_COUNT_HANDLE_H
#define LIBCPP_UTIL_REF_COUNT_HANDLE_H

#include <atomic>

namespace cpputil {

template <typename T>
class ref_count_handle {
private:
	std::atomic<T>& cnt;
public:
	ref_count_handle(std::atomic<T> &c) : cnt(c) {
		cnt.fetch_add(1, std::memory_order_relaxed);
	}
	~ref_count_handle() {
		cnt.fetch_sub(1, std::memory_order_relaxed);
	}
	ref_count_handle(ref_count_handle&&) = default;
	ref_count_handle& operator=(ref_count_handle&&) = default;
};

}
#endif
