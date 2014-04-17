//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_FIXED_ALLOCATOR_H
#define LIBCPP_UTIL_FIXED_ALLOCATOR_H

#include <cstddef>
#include <numeric_limit>

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
		chunk(const chunk&) = delete;
		chunk& operator=(const chunk&) = delete;
		chunk(chunk&&) noexcept = default;
		chunk& operator=(chunk&&) noexcept = default;

		void *allocate(std::size_t block_size);
		void deallocate(void *p, std::size_t block_size);
	};
	std::vector<chunk> storage;
	chunk *alloc;
	chunk *dealloc;
	std::size_t block_size;
	unsigned char num_blocks;

public:
	static constexpr unsigned char max_num_blocks =
	    std::numeric_limits<unsigned char>::max();

	fixed_allocator(std::size_t block_size, unsigned char num_blocks)
		: alloc(nullptr), dealloc(nullptr), block_size(block_size),
		  num_blocks(num_blocks) {
	}

	void *allocate() {
		// TODO: Allocate one block
	}

	void deallocate(void *p) {
		// TODO: Deallocate one block
	}
};

void fixed_allocator::chunk::chunk(std::size_t block_size,
				  unsigned char blocks) {
	data = new unsigned char[block_size * blocks];
	first_free_block = 0;
	num_blocks_free = blocks;
	// Initialize the in-place linked list
	unsigned char *p = data;
	for (unsigned char i = 0; i < blocks; p += block_size)
		*p = ++i;
}

void fixed_allocator::chunk::~chunk() {
	delete[] data;
}

void *fixed_allocator::chunk::allocate(std::size_t block_size) {
	if (num_blocks_free == 0)
		return nullptr;
	--num_blocks_free;
	unsigned char *ret = data[first_free_block * block_size];
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

template <typename T>
class small_object_allocator {

};

#endif
