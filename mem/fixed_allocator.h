#ifndef LIBCPP_UTIL_FIXED_ALLOCATOR_H
#define LIBCPP_UTIL_FIXED_ALLOCATOR_H

#include <cstddef>

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

public:
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
	
}

void fixed_allocator::chunk::deallocate(void *p, std::size_t block_size) {
	unsigned char *release = static_cast<unsigned char*>(p);
	// Push on the head of the list
	*release = first_free_block;
	// Now link to the next one

}
#endif
