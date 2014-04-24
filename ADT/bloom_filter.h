//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_BLOOM_FILTER_H
#define LIBCPP_UTIL_BLOOM_FILTER_H

#include <cstddef>
#include <bitset>
#include <utility>
#include <tuple>

// TODO: There has to be a less hideous way to do this.
namespace detail {
template <class T, class Bitset, class Tuple, unsigned K>
struct set_bits {
	static void do_it(const T &key, Bitset &b, Tuple &tup) {
		set_bits<T, Bitset, Tuple, K - 1>::do_it(key, b, tup);
		b.set(std::get<K - 1>(tup)(key) % b.size());
	}
};

// Base case
template <class T, class Bitset, class Tuple>
struct set_bits<T, Bitset, Tuple, 0> {
	static void do_it(const T &, Bitset &, Tuple &) {
	}
};

template <class T, class Bitset, class Tuple, unsigned K>
struct has_bits {
	static bool do_it(const T &key, Bitset &b, Tuple &tup) {
		return has_bits<T, Bitset, Tuple, K - 1>::do_it(key, b, tup) &&
		       b.test(std::get<K - 1>(tup)(key) % b.size());
	}
};
template <class T, class Bitset, class Tuple>
struct has_bits<T, Bitset, Tuple, 0> {
	static bool do_it(const T &, Bitset &, Tuple &) {
		return true;
	}
};
}

template <typename T, size_t N, class Hash = std::hash<T>, class... OtherHash>
class bloom_filter {
private:
	std::bitset<N> data_;
	std::tuple<Hash, OtherHash...> hashers_;

	constexpr static size_t num_hashers_ =
	    std::tuple_size<decltype(hashers_)>::value;

public:
	bloom_filter() = default;
	~bloom_filter() = default;
	bloom_filter(const bloom_filter&) = default;
	bloom_filter& operator=(const bloom_filter&) = default;

	bool operator==(const bloom_filter& rhs) const {
		return data_ == rhs.data_;
	}

	bool operator!=(const bloom_filter& rhs) const {
		return data_ != rhs.data_;
	}

	bloom_filter& operator|=(const bloom_filter& rhs) const {
		data_ |= rhs.data_;
		return *this;
	}

	bloom_filter& operator&=(const bloom_filter& rhs) const {
		data_ &= rhs.data_;
		return *this;
	}

	constexpr size_t size() const {
		return data_.size();
	}
	size_t count() const {
		return data_.count();
	}

	void insert(const T &key) {
		detail::set_bits<T, decltype(data_), decltype(hashers_),
				 num_hashers_>::do_it(key, data_, hashers_);
	}

	size_t count(const T &key) {
		return detail::has_bits<T, decltype(data_), decltype(hashers_),
					num_hashers_>::do_it(key, data_,
							     hashers_);
	}
};

// TODO: Non-member functions for bitwise AND and OR.

#endif
