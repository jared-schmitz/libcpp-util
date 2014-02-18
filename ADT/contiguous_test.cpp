#include  "contiguous_set.h"
#include <cstdlib>
#include <cstdio>
#include <random>
#include <string>

template <typename T>
void validate_assert(T& s) {
	if (!s.validate()) {
		puts("Loserar");
		abort();
	}
}

int main(int argc, char *argv[]) {

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<uint64_t> dist(0, 1ULL << 63);

	contiguous_set<uint64_t> s;

	puts("Putting duplicates");
	for (uint64_t i = 0; i < 10; ++i)
		s.insert(1);
	validate_assert(s);

	s.clear();

	puts("Putting sequential");
	for (uint64_t i = 0; i < 10; ++i)
		s.insert(i);

	validate_assert(s);

	s.clear();
	puts("Putting reverse sequential");
	for (uint64_t i = 10; i > 0; --i)
		s.insert(i);
	
	validate_assert(s);

	s.clear();

	puts("Bulk");
	s.insert({5, 4, 1, 4, 5, 10});
	validate_assert(s);

	if (argc == 1)
		return 0;

	puts("Iterating...");
	unsigned iterations = std::stoi(argv[1]);
	while (--iterations) {
		uint64_t random = dist(mt);
		if (random % 577 == 0) {
			// Wipe container
			s.clear();
		} else if ((random % 100 == 0) && !s.empty()) {
			// Random erase
			uint64_t pos = dist(mt) % s.size();
			s.erase(std::next(s.begin(), pos));
		} else if (random % 1234 == 0) {
			uint64_t pos1 = dist(mt);
			uint64_t pos2 = dist(mt);
			pos1 %= s.size();
			pos2 %= s.size() - pos1;
			auto I = std::next(s.begin(), pos1);
			s.erase(I, std::next(I, pos2));	
		} else if (random % 3 == 0) {
			size_t max = dist(mt) % 1000;
			std::vector<uint64_t> v(max);
			for (size_t i = 0; i < max; ++i)
				v.push_back(dist(mt));
			s.insert(v.begin(), v.end());
		} else
			s.insert(random);
		validate_assert(s);
	}
	return 0;
}
