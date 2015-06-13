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
#include <utility>

namespace cpputil {
inline int vstrprintf(std::string& s, const char* fmt, va_list ap) {
	// Conformance of implementation to sprintf semantics is paramount
	// (besides the buffer overflow parts). That said, this can be
	// implemented MUCH more efficiently. Also sorry MSVC. Come back with
	// C99.
	va_list ap2;
	va_copy(ap2, ap);
	int count = std::vsnprintf(nullptr, 0, fmt, ap);
	if (count >= 0) {
		s.resize(count + 1);
		std::vsnprintf((char*)s.data(), s.size(), fmt, ap2);
	}
	va_end(ap2);
	return count;
}

inline int vstrprintf_private(std::string& s, const char* fmt, ...) {
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vstrprintf(s, fmt, ap);
	va_end(ap);
	return ret;
}

template <typename... Ts>
int strprintf(std::string& s, const char* fmt, Ts&&... ts) {
	int count = std::vsnprintf(nullptr, 0, fmt, ts...);
	if (count >= 0) {
		s.resize(count + 1);
		std::vsnprintf((char*)s.data(), s.size(), fmt, ts...);
	}
	return count;
}

inline std::string vstrprintf(const char* fmt, va_list ap) {
	std::string ret;
	vstrprintf(ret, fmt, ap);
	return ret;
}

template <typename... Ts>
std::string astrprintf(const char* fmt, Ts&&... ts) {
	std::string ret;
	vstrprintf_private(ret, fmt, std::forward<Ts>(ts)...);
	return ret;
}

}
#endif
