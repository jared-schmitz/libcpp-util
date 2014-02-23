#ifndef LIBCPP_UTIL_FIXED_ALLOCATOR_H
#define LIBCPP_UTIL_FIXED_ALLOCATOR_H

#include <cstddef>
#include <numeric_limit>

class fixed_allocator {
private:
	struct chunk {
		unsigned char *data;
		unsigned char first_free_block;
		unsigned char num_blocks_free;

		void init(std::size_t block_size, unsigned char blocks);
		void destroy();
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
		num_blocks(num_blocks) {}

	void *allocate(std::size_t n) {
	}

	void deallocate(void *p, std::size_t n) {
	}	
};

void fixed_allocator::chunk::init(std::size_t block_size,
				  unsigned char blocks) {
	data = new unsigned char[block_size * blocks];
	first_free_block = 0;
	num_blocks_free = blocks;
	// Initialize the in-place linked list
	unsigned char *p = data;
	for (unsigned char i = 0; i < blocks; p += block_size)
		*p = ++i;
}

void fixed_allocator::chunk::destroy() {
	delete data;
}

void *fixed_allocator::chunk::allocate(std::size_t block_size) {
	if (num_blocks_free == 0)
		return nullptr;
	--num_blocks_free;
	unsigned char* ret = data[first_free_block * block_size];
	first_free_block = *ret; 
	return ret;
}

void fixed_allocator::chunk::deallocate(void *p, std::size_t block_size) {
	unsigned char *release = static_cast<unsigned char*>(p);
	// Link to the head
	*release = first_free_block;
	// Make the head point here
	first_free_block = static_cast<unsigned char>((release - data) /
			block_size);
	++num_blocks_free;
}

// A type-aware wrapper to fixed_allocator to make standard containers smile.
template <typename T>
class fixed_type_allocator : public fixed_allocator {
public:
	typedef T value_type;
	fixed_type_allocator() : fixed_allocator(sizeof(T),
			fixed_allocator::max_num_blocks) {}
	~fixed_type_allocator() = default;

	// TODO: Need to be able to rebind, which requires reinitializing the
	// block-size and what not. Also need to write magic casting that wraps
	// allocate/deallocate.
};
#endif
