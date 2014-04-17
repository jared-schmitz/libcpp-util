//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_ARRAY_REF_H
#define LIBCPP_UTIL_ARRAY_REF_H

#include <iterator>
#include <cstddef>
#include <vector>
#include <array>
#include <limits>
#include <stdexcept>

// TODO: This class has been to changed to array_view and is now far more
// complicated than something I'm interested in implementing. Delete?

// Represents a constant reference to an array. Let's you treat various styles
// of contiguous storage uniformly (e.g. not having to have 1 million
// overloads). This class knows how to deal with std::array, std::vector,
// C-style arrays, pointer ranges, and pointer/length pairs. The array_ref
// does not own the underlying storage, so track memory usage carefully. This
// class is to be included in C++14 AFAIK and I have attempted to keep the
// interface the same to make the transition seamless.
template <typename T>
class array_ref {
public:
	using value_type = T;
	using pointer = const T*;
	using reference = const T&;
	using const_reference = const T&;
	using iterator = const T*;
	using const_iterator = iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = reverse_iterator;
	using size_type = size_t;
	using difference_type = ptrdiff_t;

private:
	const T* start;
	size_t len;
public:
	array_ref()
		: start(nullptr), len(0) {}
	array_ref(const T& elem)
		: start(&elem), len(1) {}
	array_ref(const T* data, size_t length)
		: start(data), len(length) {}
	array_ref(const T* data, const T* end)
		: start(data), len(end - start) {}

	template <typename Alloc>
		array_ref(const std::vector<T, Alloc>& v)
		: start(v.data()), len(v.size()) {}

	template <size_t N>
		array_ref(const T (&arr)[N]) : start(arr), len(N) {}

	template <size_t N>
		array_ref(const std::array<T, N>& arr) : start(arr), len(N) {}

	array_ref(const array_ref&) = default;
	array_ref& operator=(const array_ref&) = default;

	array_ref slice(size_type pos, size_type n = size_type(-1)) const {
		// If n == -1, it's the whole string.
		return array_ref(start + pos,
				(n == size_type(-1) ? n : len - pos));
	}

	// Container access
	const_iterator begin() const {
		return start;
	}
	const_iterator end() const {
		return start + len;
	}
	const_iterator cbegin() const {
		return start;
	}
	const_iterator cend() const {
		return start + len;
	}
	const_reverse_iterator rbegin() const {
		return reverse_iterator(end());
	}
	const_reverse_iterator rend() const {
		return reverse_iterator(begin());
	}
	const_reverse_iterator crbegin() const {
		return reverse_iterator(cend());
	}
	const_reverse_iterator crend() const {
		return reverse_iterator(cbegin());
	}

	const T* data() const {
		return start;
	}

	// Capacity/Size
	bool empty() {
		return len == 0;
	}
	size_t size() const {
		return len;
	}
	constexpr size_type max_size() const {
		return std::numeric_limits<size_type>::max();
	}

	// Element access
	const T& front() const {
		return *start;
	}
	const T& back() const {
		return start[len - 1];
	}

	const T& operator[](size_t idx) const {
		return start[idx];
	}
	const T& at(size_t idx) const {
		return idx >= size() ?
			throw std::out_of_range("at() index out of range")
			: start[idx];
	}

	// Explicit conversion
	explicit operator std::vector<T>() const {
		return std::vector<T>(begin(), end());
	}
	std::vector<T> vec() const {
		return std::vector<T>(*this);
	}

	// Mutators
	void clear() {
		start = nullptr; len = 0;
	}
	void remove_prefix(size_type n) {
		start += n;
		len -= n;
	}
	void remove_suffix(size_type n) {
		len -= n;
	}
	void pop_front() {
		remove_prefix(1);
	}
	void pop_back() {
		remove_suffix(1);
	}
};

// Convenience functions that will deduce the type of T
template <typename T>
array_ref<T> make_array_ref(const T& elem) {
	return elem; /* Implicitly built */
}

template <typename T>
array_ref<T> make_array_ref(const T* data, size_t length) {
	return array_ref<T>(data, length);
}

template <typename T>
array_ref<T> make_array_ref(const T* first, const T* last) {
	return array_ref<T>(first, last);
}

template <typename T>
array_ref<T> make_array_ref(const std::vector<T> v) {
	return v; /* Implicitly constructed */
}

template <typename T, size_t N>
array_ref<T> make_array_ref(const T (&arr)[N]) {
	return array_ref<T>(arr);
}

template <typename T, size_t N>
array_ref<T> make_array_ref(const std::array<T, N>& arr) {
	return array_ref<T>(arr);
}
#endif
