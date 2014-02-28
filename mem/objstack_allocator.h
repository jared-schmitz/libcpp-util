#ifndef LIBCPP_UTIL_OBJSTACK_ALLOCATOR_H
#define LIBCPP_UTIL_OBJSTACK_ALLOCATOR_H

#include <cstddef>
#include <memory>

template <unsigned N>
class objstack {
private:
	unsigned char storage[N];
	std::size_t cursor;

public:
	objstack() : cursor(0) {
	}
	void *allocate(std::size_t n) {
		if (cursor > N - n)
			return nullptr;
		void *ret = storage[cursor];
		cursor += n;
		return ret;
	}
};

template <typename T, unsigned N>
class objstack_allocator {
private:
	std::shared_ptr<objstack<N * sizeof(T)>> stack;

public:
	typedef T value_type;
	objstack_allocator() : stack(std::make_shared()) {
	}
	objstack_allocator(const objstack_allocator &) = default;
	objstack_allocator(objstack_allocator &&) = default;
	objstack_allocator &operator=(const objstack_allocator &) = default;
	objstack_allocator &operator=(objstack_allocator &&) = default;
	~objstack_allocator() = default;

	template <typename U>
	objstack_allocator(const objstack_allocator<U> &other)
		: stack(other.stack) {
	}

	T *allocate(std::size_t n) {
		if (T *ptr = static_cast<T*>(stack->allocate(n)))
			return ptr;
		return nullptr; // TODO: Fallback
	}
	void deallocate(T *, std::size_t) {
		// Noop!
	}
};

template <typename T, unsigned N, typename U, unsigned M>
bool operator==(const objstack_allocator<T, N> &a,
		const objstack_allocator<U, M> &b) {
	return N == M && a.stack == b.stack;
}

template <typename T, unsigned N, typename U, unsigned M>
bool operator!=(const objstack_allocator<T, N> &a,
		const objstack_allocator<U, M> &b) {
	return !(a == b);
}

#endif
