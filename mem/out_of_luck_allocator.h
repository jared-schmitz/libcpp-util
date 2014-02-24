#ifndef LIBCPP_UTIL_OUT_OF_LUCK_ALLOCATOR_H
#define LIBCPP_UTIL_OUT_OF_LUCK_ALLOCATOR_H

#include <cstddef>
#include <new>

// This simple class has two main uses. The more common one would be to
// terminate an allocator chain, where we have finally decided we're out of
// memory. The more obscure usage would be for testing purposes, to see how
// containers handle an out-of-memory allocation (exception guarantees, etc.),
// the same way /dev/full would be used to stress-test a file utility.
template <typename T, bool exceptions = true>
class out_of_luck_allocator {
public:
	typedef T value_type;
	out_of_luck_allocator() = default;
	template <typename U>
	out_of_luck_allocator(const out_of_luck_allocator<U>& other) {}
	T *allocate(std::size_t, T* unused = 0) {
		(void)unused;
		return exceptions : throw std::bad_alloc("Out of memory") :
			nullptr;
	}
	void deallocate(T*, std::size_t) {}
};
#endif
