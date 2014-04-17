//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_FIXED_ALLOCATOR_H
#define LIBCPP_UTIL_FIXED_ALLOCATOR_H

#include "libcpp-util/ADT/contiguous_set.h"
#include "libcpp-util/mem/util.h"

#include <algorithm>
#include <cassert>
#include <cstddef>

// This allocation scheme is taken from Andrei Alexandrescu's Modern C++ Design.
// I have tweaked a few things to make it fit better with the C++11 allocation
// model, as well as use some convenient C++11 features. Mainly, we can afford
// to use a value constructor for chunk, because we have rvalue references and
// perfect forwarding.
class fixed_allocator {
private:
	struct chunk {
		unsigned char *data;
		unsigned char first_free_block;
		unsigned char num_blocks_free;

		chunk(std::size_t block_size, unsigned char blocks);
		~chunk();
		chunk(const chunk &) = delete;
		chunk &operator=(const chunk &) = delete;
		chunk(chunk &&o) noexcept
		    : data(o.data),
		      first_free_block(o.first_free_block),
		      num_blocks_free(o.num_blocks_free) {
			o.data = nullptr;
		};
		chunk &operator=(chunk &&o) noexcept {
			data = o.data;
			first_free_block = o.first_free_block;
			num_blocks_free = o.num_blocks_free;
			o.data = nullptr;
			return *this;
		}

		void *allocate(std::size_t block_size);
		void deallocate(void *p, std::size_t block_size);
	};
	std::vector<chunk> storage;
	chunk *alloc;
	chunk *dealloc;
	std::size_t block_size;
	unsigned char num_blocks;
	size_t num_blocks_free;

	chunk *get_next_block_to_allocate_from();
	chunk *get_block_to_deallocate_from(void *p, size_t block_size);

	bool chunk_contains(const chunk *c, const void *p) const {
		return p >= c->data && p < (c->data + num_blocks * block_size);
	}

public:
	std::size_t get_block_size() const {
		return block_size;
	}

	fixed_allocator(std::size_t block_size, unsigned char num_blocks)
		: alloc(nullptr), dealloc(nullptr), block_size(block_size),
		  num_blocks(num_blocks), num_blocks_free(0) {
	}

	void *allocate() {
		chunk *c = get_next_block_to_allocate_from();
		--num_blocks_free;
		return c->allocate(block_size);
	}

	void deallocate(void *p) {
		chunk *c = get_block_to_deallocate_from(p, block_size);
		assert(c && "No block to deallocate from");
		++num_blocks_free;
		c->deallocate(p, block_size);
	}
};

fixed_allocator::chunk::chunk(std::size_t block_size, unsigned char blocks) {
	data = new unsigned char[block_size * blocks];
	first_free_block = 0;
	num_blocks_free = blocks;
	// Initialize the in-place linked list
	unsigned char *p = data;
	for (unsigned char i = 0; i < blocks; p += block_size)
		*p = ++i;
}

fixed_allocator::chunk::~chunk() {
	delete[] data;
}

void *fixed_allocator::chunk::allocate(std::size_t block_size) {
	if (num_blocks_free == 0)
		return nullptr;
	--num_blocks_free;
	// Get the block to return
	unsigned char *ret = &data[first_free_block * block_size];
	// Mark the first free block as the next block of this one
	first_free_block = *ret;
	return ret;
}

void fixed_allocator::chunk::deallocate(void *p, std::size_t block_size) {
	unsigned char *release = static_cast<unsigned char *>(p);
	// Link to the head
	*release = first_free_block;
	// Make the head point here
	first_free_block =
	    static_cast<unsigned char>((release - data) / block_size);
	++num_blocks_free;
}

fixed_allocator::chunk *fixed_allocator::get_next_block_to_allocate_from() {
	// See if the hot block has space
	if (alloc && alloc->num_blocks_free)
		return alloc;
	// See if any blocks have space
	if (num_blocks_free) {
		for (auto &I : storage) {
			if (I.num_blocks_free) {
				alloc = &I;
				return alloc;
			}
		}
	}
	// Allocate a new block. If we're going te resize, we also need to make
	// sure to fix-up the dealloc pointer as well.
	size_t dealloc_index =
	    dealloc ? std::distance(&storage.front(), dealloc) : 0;
	storage.emplace_back(block_size, num_blocks);
	dealloc = &storage[dealloc_index];
	num_blocks_free += num_blocks;
	alloc = &storage.back();
	return alloc;
}

fixed_allocator::chunk *
fixed_allocator::get_block_to_deallocate_from(void *p, size_t block_size) {
	if (dealloc && chunk_contains(dealloc, p))
		return dealloc;
	for (auto &I : storage) {
		if (chunk_contains(&I, p)) {
			dealloc = &I;
			return &I;
		}
	}
	assert(0 && "Trying to deallocate invalid pointer");
}

class small_object_allocator_base {
private:
	struct block_size_compare {
		bool operator()(const fixed_allocator &a,
				const fixed_allocator &b) const {
			return a.get_block_size() < b.get_block_size();
		}
	};
	struct direct_size_compare {
		bool operator()(const fixed_allocator &a, size_t b) const {
			return a.get_block_size() < b;
		}
	};
	// This is kept sorted on the sizes of the contained blocks
	contiguous_set<fixed_allocator, block_size_compare> allocators;
	using iterator = decltype(allocators)::iterator;

	fixed_allocator *alloc, *dealloc;

	fixed_allocator *
	get_allocator_for_block_size(size_t block_size,
				     fixed_allocator *hint = 0) {
		// If we don't have a hint, just search for it directly.
		if (!hint)
			return &*std::lower_bound(allocators.begin(),
						  allocators.end(), block_size,
						  direct_size_compare());
		// See if the hint is correct.
		if (hint->get_block_size() == block_size)
			return hint;

		// If it's not the right size, binary search along the correct
		// side.
		iterator first, last;
		if (hint->get_block_size() < block_size) {
			first = iterator(hint);
			last = allocators.end();
		} else {
			first = allocators.begin();
			last = iterator(hint);
		}
		return &*std::lower_bound(first, last, block_size,
					  direct_size_compare());
	}

public:
	void add_storage_size(size_t block_size) {
		// FIXME: Fix-up the alloc/dealloc pointers if defined.
		allocators.emplace(block_size, 255);
	}
	void *allocate(size_t block_size) {
		alloc = get_allocator_for_block_size(block_size, alloc);
		return alloc->allocate();
	}
	void deallocate(void *p, size_t block_size) {
		dealloc = get_allocator_for_block_size(block_size, dealloc);
		dealloc->deallocate(p);
	}

	static small_object_allocator_base &get() {
		static small_object_allocator_base impl;
		return impl;
	}
};

template <typename T>
class small_object_allocator : public no_cxx11_allocators<T> {
private:
	small_object_allocator_base &base;

public:
	typedef T value_type;

	template <typename U>
	struct rebind {
		typedef small_object_allocator<U> other;
	};
	small_object_allocator() : base(small_object_allocator_base::get()) {
		// TODO: Singleton for base?
		base.add_storage_size(sizeof(T));
	}
	template <typename U>
	small_object_allocator(const small_object_allocator<U> &o) {
		base.add_storage_size(sizeof(T));
	}

	T *allocate(size_t n, const T * = 0) {
		if (n > 1)
			return static_cast<T *>(::operator new(sizeof(T) * n));
		else
			return static_cast<T *>(base.allocate(sizeof(T)));
	}

	void deallocate(T *p, size_t n) {
		if (n > 1)
			::operator delete(p);
		else
			base.deallocate(p, sizeof(T));
	}
};

#endif
