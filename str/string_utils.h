//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_STRING_UTILS_H
#define LIBCPP_UTIL_STRING_UTILS_H

#include "libcpp-util/cxx14/string_ref.h"
#include <string>

namespace cpputil {

size_t string_total_len(string_ref s) {
	return s.size();
}

template <class... Strings>
size_t string_total_len(string_ref s, Strings&&... args) {
	return s.size() + string_total_len(args...);
}

namespace {
void __concat_in_place(std::string& s1, string_ref s2) {
	s1.append(s2.data(), s2.size());
}
template <class... Strings>
void __concat_in_place(std::string& s1, string_ref s2, Strings&&... args) {
	s1.append(s2.data(), s2.size());
	__concat_in_place(s1, args...);
}
}

template <class... Strings>
void concat_in_place(std::string& s1, string_ref s2, Strings&&... args) {
	s1.reserve(string_total_len(s1, s2, args...));
	__concat_in_place(s1, s2, args...);
}

template <class... Strings>
std::string concat(string_ref s, Strings&&... args) {
	std::string ret;
	concat_in_place(ret, s, args...);
	return ret;
}

}
#endif
