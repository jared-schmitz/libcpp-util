#include <set>
#include "SlabAllocator.hpp"
int main() {
	std::set<int, std::less<int>, slab_allocator<int>> s;

	unsigned j = 4;
	while (j--) {
		s.clear();
		for (unsigned i = 0; i < 100000; ++i) {
			s.insert(i);
		}
	}
	return 0;
}
