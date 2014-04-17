//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_MALLOC_ALLOCATOR_H
#define LIBCPP_UTIL_MALLOC_ALLOCATOR_H

#include "libcpp-util/mem/util.h"

#include <cstdlib>

template <typename T>
class malloc_allocator : public no_cxx11_allocators<T> {
public:
	typedef T value_type;
	T *allocate(size_t n, const void* = 0) {
		void *p = std::malloc(n * sizeof(T));
		if (!p && n != 0)
			abort();
		return static_cast<T*>(p);
	}
	void deallocate(T *p, size_t) {
		std::free(p);
	}

	malloc_allocator() = default;
	template <typename U>
	malloc_allocator(const malloc_allocator<U>&) { }
	malloc_allocator(const malloc_allocator&) = default;
	~malloc_allocator() = default;

	template <typename U>
	struct rebind {
		using other = malloc_allocator<U>;
	};
};

#endif
