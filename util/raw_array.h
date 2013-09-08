//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_RAW_ARRAY_H
#define LIBCPP_UTIL_RAW_ARRAY_H

#include <cstddef>
#include <type_traits>

namespace cpputil {

template <class T, size_t N, size_t alignment = std::alignment_of<T>::value>
class raw_array {
private:
	using data_type = typename 
		std::aligned_storage<sizeof(T), alignment>::type;

	data_type data[N];
public:
	raw_array() = default;
	// TODO: Nice value constructors, initializer-lists, etc
	raw_array(const raw_array&) = default;
	raw_array& operator=(const raw_array&) = default;
	~raw_array() = default;

	constexpr size_t capacity() const {
		return N;
	}

	const T& operator[](size_t index) const {
		return reinterpret_cast<const T&>(data[index]);
	}

	T& operator[](size_t index) {
		return reinterpret_cast<T&>(data[index]);
	}
};

}

#endif
