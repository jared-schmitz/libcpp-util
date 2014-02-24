#include <unordered_set>
#include <set>
#include <iostream>
#include <fstream>

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "usage: %s letters dictionary_file.txt\n",
				argv[0]);
		return -1;
	}
	std::ifstream in(argv[2]);
	if (!in) {
		fprintf(stderr, "Couldn't open %s\n", argv[2]);
		return -1;
	}

	// Read in the entire dictionary
	using string_iter = std::istream_iterator<std::string>;
	std::unordered_map words(string_iter(in), string_iter());


	// For each anagram, if it's in the dictionary save it.
	std::set<std::string> words;
	std::string letters(argv[1]);
	std::sort(letters.begin(), letters.end());
	do {
		

	} while (std::next_permutation(letters.begin(), letters.end()));


	// Now print them by length.
	return 0;
}
