//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_STRPRINTF_H
#define LIBCPP_UTIL_STRPRINTF_H

#include "libcpp-util/cxx14/string_ref.h"
#include <string>
#include <cstdarg>
#include <cstdio>

namespace cpputil {

int vstrprintf(std::string& s, string_ref fmt, va_list ap) {
	// Conformance of implementation to sprintf semantics is paramount
	// (besides the buffer overflow parts). That said, this can be
	// implemented MUCH more efficiently. Also sorry MSVC. Come back with
	// C99.
	va_list ap2;
	va_copy(ap2, ap);
	int count = std::vsnprintf(nullptr, 0, fmt.data(), ap);
	if (count != -1) {
		s.resize(count + 1);
		std::vsnprintf((char*)s.data(), s.size(), fmt.data(), ap2);
	}
	va_end(ap2);
	return count;
}

std::string vstrprintf(string_ref fmt, va_list ap) {
	std::string ret;
	vstrprintf(ret, fmt, ap);
	return ret;
}

int strprintf(std::string& s, string_ref fmt, ...) {
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vstrprintf(s, fmt, ap);
	va_end(ap);
	return ret;
}

std::string astrprintf(string_ref fmt, ...) {
	va_list ap;
	std::string ret;
	va_start(ap, fmt);
	vstrprintf(ret, fmt, ap);
	va_end(ap);
	return ret;
}

}
#endif
