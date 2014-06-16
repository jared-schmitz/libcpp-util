//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_OBJSTACK_ALLOCATOR_H
#define LIBCPP_UTIL_OBJSTACK_ALLOCATOR_H

#include "libcpp-util/mem/util.h"

#include <cstddef>
#include <new>
#include <memory>
#include <limits>

template <unsigned N>
class fixed_objstack {
private:
	unsigned char storage[N];
	std::size_t size;
public:
	fixed_objstack() : size(N) {
	}

	void *allocate(std::size_t n, std::size_t alignment) {
		void *tmp = storage + (N - size);
		if (!align(alignment, n, tmp, size))
			return nullptr;
		size -= n;
		return tmp;
	}

	constexpr size_t max_size() const {
		return N;
	}
};

template <unsigned N>
class objstack {
private:
	struct objstack_node : public fixed_objstack<N> {
		std::unique_ptr<objstack_node> next;

		objstack_node(std::unique_ptr<objstack_node> &&n) :
			next(std::move(n)) {
		}
	};

	using link = std::unique_ptr<objstack_node>;
	link head;

	void allocate_new_node() {
		auto new_head = link(new objstack_node(std::move(head)));
		head = std::move(new_head);
	}

public:
	void *allocate(std::size_t n, std::size_t alignment) {
		if (!head)
			allocate_new_node();
		void *ret = head->allocate(n, alignment);
		if (ret)
			return ret;
		allocate_new_node();
		return head->allocate(n, alignment);
	}

	constexpr size_t max_size() const {
		size_t t = std::numeric_limits<size_t>::max();
		return t - (t % N);
	}
};

template <typename T, typename Stack>
class objstack_alloc_base : public no_cxx11_allocators<T> {
private:
	std::shared_ptr<Stack> stack;

public:
	template <typename U, typename S>
	friend class objstack_alloc_base;
	typedef T value_type;

	objstack_alloc_base() : stack(std::make_shared<Stack>()) {
	}
	objstack_alloc_base(objstack_alloc_base &&) noexcept = default;
	objstack_alloc_base(const objstack_alloc_base &) noexcept = default;
	objstack_alloc_base &
	operator=(objstack_alloc_base &&) noexcept = default;
	objstack_alloc_base &
	operator=(const objstack_alloc_base &) noexcept = default;
	~objstack_alloc_base() = default;

	template <typename U>
	struct rebind {
		typedef objstack_alloc_base<U, Stack> other;
		// TODO: Need a static_assert here too.
	};

	template <typename U>
	objstack_alloc_base(const objstack_alloc_base<U, Stack> &other) noexcept
	    : stack(other.stack) {
		static_assert(sizeof(T) <= other.max_size(), 
				"Can't allocate objects of type T from rhs");
	}

	T *allocate(std::size_t n, T *hint = 0) {
		std::size_t bytes = sizeof(T) * n;
		if (bytes > max_size())
			return static_cast<T*>(std::malloc(bytes));
		if (T *ptr = static_cast<T *>(stack->allocate(bytes,
			std::alignment_of<T>::value)))
			return ptr;
		return nullptr;
	}
	void deallocate(T *p, std::size_t n) {
		if (n * sizeof(T) > max_size())
			std::free(p);
	}

	std::size_t max_size() const {
		return stack->max_size();
	}
};

template <typename T, typename U, typename Stack>
bool operator==(const objstack_alloc_base<T, Stack> &a,
		const objstack_alloc_base<U, Stack> &b) {
	return a.stack == b.stack;
}

template <typename T, typename U, typename Stack>
bool operator!=(const objstack_alloc_base<T, Stack> &a,
		const objstack_alloc_base<U, Stack> &b) {
	return !(a == b);
}

template <typename T, unsigned N>
using objstack_allocator = objstack_alloc_base<T, objstack<N>>;

template <typename T, unsigned N>
using fixed_objstack_allocator = objstack_alloc_base<T, fixed_objstack<N>>;

#endif
