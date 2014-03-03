#ifndef LIBCPP_UTIL_OBJSTACK_ALLOCATOR_H
#define LIBCPP_UTIL_OBJSTACK_ALLOCATOR_H

#include <cstddef>
#include <new>
#include <memory>

template <unsigned N>
class objstack {
private:
	unsigned char storage[N];
	unsigned size;
public:
	objstack() : size(N) {}

	void *allocate(std::size_t n, std::size_t alignment) {
		return std::align(n, alignment, storage + (N - size), size);
	}
};

template <typename T, unsigned N>
class objstack_allocator {
private:
	std::shared_ptr<objstack<N>> stack;
public:
	typedef T value_type;
	objstack_allocator() : stack(std::make_shared()) {}
	objstack_allocator(objstack_allocator&&) noexcept = default;
	objstack_allocator(const objstack_allocator&) noexcept = default;
	objstack_allocator& operator=(objstack_allocator&&) noexcept = default;
	objstack_allocator& operator=(const objstack_allocator&) noexcept = default;
	~objstack_allocator() = default;

	template <typename U, unsigned N>
	objstack_allocator(const objstack_allocator<U, N>& other) noexcept
       	: stack(other.stack) {} 

	T* allocate(std::size_t n, T* hint = 0) {
		if (T* ptr = stack.allocate(sizeof(T) * n,
					std::alignment_of<T>::value))
			return ptr;
		return nullptr;
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
