#ifndef LIBCPP_UTIL_OBJSTACK_ALLOCATOR_H
#define LIBCPP_UTIL_OBJSTACK_ALLOCATOR_H

#include <cstddef>
#include <new>

template <typename T, unsigned N>
class objstack_allocator {
private:
	unsigned char *storage;
	std::size_t cursor;
public:
	typedef T value_type;
	objstack_allocator() : storage(new unsigned char[N]), cursor(0) {}
	~objstack_allocator() {
		delete storage;
	}

	template <typename U>
	objstack_allocator(const objstack_allocator<U>& other);

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
