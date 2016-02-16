//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_ARENA_ALLOCATOR_H
#define LIBCPP_UTIL_ARENA_ALLOCATOR_H

#include "libcpp-util/mem/util.h"

#include <cstddef>
#include <new>
#include <memory>
#include <limits>

// FIXME: Hoist this elsewhere. Probably util.h
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

class arena {
private:
	unsigned char *storage;
	std::size_t size;
public:
	arena(size_t N) : size(N) { }

	void *allocate(std::size_t n, std::size_t alignment) {
		if (!storage) {
			// TODO: Need alignment to be a multiple of size if
			// allocating
			storage = aligned_alloc(PAGE_SIZE, size);
			if (!storage)
				return nullptr;
		}
		void *tmp = storage + (N - size);
		if (!align(alignment, n, tmp, size))
			return nullptr;
		size -= n;
		return tmp;
	}

	constexpr size_t max_size() const {
		return std::numeric_limits<std::size_t>::max();
	}
};

class arena_allocator_base {
private:
	struct arena_node : public arena {
		std::unique_ptr<arena_node> next;

		arena_node(std::unique_ptr<arena> &&n, std::size_t sz) :
			arena(sz), next(std::move(n)) {
		}
	};

	using link = std::unique_ptr<arena_node>;
	link head;

	void allocate_new_node(std::size_t sz) {
		if (sz < (1 << 20))
			sz *= 4;
		// XXX: Exception-safety
		auto new_head = link(new arena_node(sz, std::move(head)));
		head = std::move(new_head);
	}

public:
	void *allocate(std::size_t n, std::size_t alignment) {
		if (!head)
			allocate_new_node(n);
		void *ret = head->allocate(n, alignment);
		if (ret)
			return ret;
		allocate_new_node(n);
		return head->allocate(n, alignment);
	}

	constexpr size_t max_size() const {
		return std::numeric_limits<size_t>::max();
	}
};

template <typename T>
class arena_allocator : public no_cxx11_allocators<T> {
private:
	arena_allocator_base& base;
public:
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
		typedef arena_allocator<U> other;
	};

	template <typename U>
	arena_allocator(const arena_allocator<U> &other) noexcept
	    : base(arena_allocator_base::get()) {
	}

	T *allocate(std::size_t n, T* = 0) {
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

template <typename T, typename U>
bool operator==(const arena_allocator_base<T> &a,
		const arena_allocator_base<U> &b) {
	return a.stack == b.stack;
}

template <typename T, typename U>
bool operator!=(const arena_allocator_base<T> &a,
		const arena_allocator_base<U> &b) {
	return !(a == b);
}

#undef PAGE_SIZE
#endif
