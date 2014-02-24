#ifndef LIBCPP_UTIL_OBJSTACK_ALLOCATOR_H
#define LIBCPP_UTIL_OBJSTACK_ALLOCATOR_H

#include <cstddef>
#include <new>

// internal_allocator holds all of its memory inside the allocator itself. This
// has many implications for container performance, generally because
// traditional standard library implementations assume that copying an allocator
// is cheap. For container types which allocate an internal type (e.g. a node)
// that the templated type is stored inside, a rebound allocator may be repeated
// constructed and destroyed.
template <typename T, unsigned N>
class internal_allocator {
private:
	unsigned char storage[N];
	std::size_t cursor;
public:
	typedef T value_type;
	internal_allocator() : cursor(0) {}
	~internal_allocator() { }

	template <typename U>
	internal_allocator(const internal_allocator<U>& other) {
		// TODO: What ... are the semantics of this? Do we copy the
		// memory? Containers will copy the elements themselves so I'm
		// not sure this shouldn't be a noop.
	}

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
