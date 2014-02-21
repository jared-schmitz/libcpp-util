#ifndef LIBCPP_UTIL_OBJSTACK_ALLOCATOR_H
#define LIBCPP_UTIL_OBJSTACK_ALLOCATOR_H

#include <cstddef>
#include <new>

template <typename T, unsigned N>
class objstack_allocator {
private:
	unsigned char storage[N];
	std::size_t cursor;
public:
	typedef T value_type;
	objstack_allocator() : cursor(0) {}

	T* allocate(std::size_t n) {
		T* ret = static_cast<T*>(storage[cursor]);
		cursor += n * sizeof(T);
		if (cursor > N)
			throw std::bad_alloc("Out of objects");
		return ret;
	}
	void deallocate(T*, std::size_t) {
		// Noop!
	}
};

#endif
