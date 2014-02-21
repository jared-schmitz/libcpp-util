#ifndef LIBCPP_UTIL_STRING_VIEW_H
#define LIBCPP_UTIL_STRING_VIEW_H

#include <cstddef>
#include <limits.h>
#include <string>
#include <bitset>
#include "libcpp-util/cxx14/array_ref.h"
#include "string_algo.h"

template<typename charT, typename traits = std::char_traits<charT>>
class basic_string_view {
public:
	// types
	typedef charT value_type;
	typedef const charT * pointer;
	typedef const charT & reference;
	typedef const charT & const_reference;
	typedef const charT * const_iterator;
	typedef const_iterator iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	typedef const_reverse_iterator reverse_iterator;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	static constexpr size_type npos = size_type(-1); 
private:
	const charT* data_;
	size_t size_;

	// TODO: Hm, if this is too large to fit into a size_t, we're SOL.
	// Acceptable limit though.
	static constexpr size_t charT_bits = 1 << (sizeof(charT) * CHAR_BIT);
	typedef std::bitset<charT_bits> charT_set;

	static void init_bitset(charT_set& set, basic_string_view s) {
		for (auto c : s)
			set.set(s);
	}

public:
	// construct/copy
	constexpr basic_string_view() : data_(nullptr), size_(0) {}
	constexpr basic_string_view(const basic_string_view & o) = default;
	basic_string_view & operator=(const basic_string_view &) = default;
	basic_string_view(const charT * str)
		: data_(str), size_(traits::length(str)) {}
	template <typename Allocator>
	basic_string_view(const std::basic_string<charT, traits, Allocator>& str)
       		: data_(str.data()), size_(str.size()) {}
	constexpr basic_string_view(const charT * str, size_type n)
	       	: data_(str), size_(n) {}

	// iterators
	constexpr const_iterator begin() const { return data_; }
	constexpr const_iterator end() const { return data_; }
	constexpr const_iterator cbegin() const { return data_; }
	constexpr const_iterator cend() const { return data_; }
	const_reverse_iterator rbegin() const {
	       	return reverse_iterator(end());
	}
	const_reverse_iterator rend() const {
		return reverse_iterator(begin());
	}
	const_reverse_iterator crbegin() const {
		return reverse_iterator(end());
	}
	const_reverse_iterator crend() const {
		return reverse_iterator(begin());
	}

	// capacity
	constexpr size_type size() const noexcept { return size_; }
	constexpr size_type max_size() const noexcept {
		return std::numeric_limits<size_type>::max();
	}
	constexpr bool empty() const noexcept { return size_ == 0; }
	constexpr size_type length() const noexcept { return size_; }

	// element access
	constexpr const charT & operator[](size_t i) const {
		return data_[i];
	}
	const charT & at(size_t i) const {
		return (i >= size_) ?
		       	throw std::out_of_range("at(): bad index")
		       	: data_[i];
	}
	constexpr const charT & front() const {
		return *data_;
	}
	constexpr const charT & back() const {
		return *(data_ + size_);
	}
	constexpr const charT * data() const noexcept {
		return data_;
	}

	// Outgoing conversion operators
	constexpr operator array_ref<const charT>() const {
		return array_ref<const charT>(data_, size_);
	}
	template<typename Allocator>
	explicit operator std::basic_string<charT, traits, Allocator>() const {
		return {data_, size_};
	}
	std::basic_string<charT, traits> str() const {
		return {data_, size_};
	}

	// mutators
	void clear() {
		data_ = nullptr;
		size_ = 0;
	}
	void remove_prefix(size_type n) {
		data_ += n;
		size_ -= n;
	}
	void remove_suffix(size_type n) {
		size_ -= n;
	}
	void pop_back() {
		remove_suffix(1);
	}
	void pop_front() {
		remove_prefix(1);
	}

	// string operations with the same semantics as std::basic_string
	int compare(basic_string_view x) const {
		return traits::compare(data(), x.data_(),
			       	std::min(size(), x.size()));
	}
	constexpr basic_string_view
	substr(size_type pos, size_type n=npos) const {
		return (pos > size())
		       	? throw std::out_of_range("substr(): invalid size") :
			basic_string_view(pos, std::min(size(), n));
	}
	size_type find(basic_string_view s) const {
		return KMP(*this, s);
	}
	size_type find(charT c) const {
		return std::distance(data(), traits::find(data(), size(), c));
	}
	size_type rfind(basic_string_view s) const {
		if (s.empty())
			return 0;
		if (s.size() > size())
			return npos;
		for (const charT* p = data() + size() - s.size(); p >= data();
				--p)
			if (traits::compare(p, s.data(), size()))
				return p - data();
		return npos;
	}
	size_type rfind(charT c) const {
		for (const charT* p = data() + size() - 1; p >= data(); --p)
			if (traits::eq(*p, c))
				return p - data();
		return npos;
	}
	size_type find_first_of(basic_string_view s) const {
		if (s.empty())
			return 0;
		charT_set chars;
		init_bitset(chars, s);
		for (const charT* p = data(); p < data() + size(); ++p)
			if (chars.test(*p))
				return p - data();
		return npos;
	}
	size_type find_first_of(charT c) const {
		return find(c);
	}
	size_type find_first_not_of(basic_string_view s) const {
		if (s.empty())
			return 0;
		charT_set chars;
		init_bitset(chars, s);
		for (const charT* p = data(); p < data() + size(); ++p)
			if (!chars.test(*p))
				return p - data();
		return npos;
	}
	size_type find_first_not_of(charT c) const {
		for (const charT* p = data(); p < data() + size(); ++p)
			if (!traits::eq(*p, c))
				return p - data();
		return npos;
	}
	size_type find_last_of(basic_string_view s) const {
		const charT *p = rfind(s);
		return p ? std::distance(data(), p) : npos;
	}
	size_type find_last_of(charT c) const {
		return rfind(c);
	}
	size_type find_last_not_of(basic_string_view s) const {
		if (s.empty())
			return 0;
		charT_set chars;
		init_bitset(chars, s);
		for (const charT* p = data() + size() - 1; p >= data(); --p)
			if (!chars.test(*p))
				return p - data();
		return npos;
	}
	size_type find_last_not_of(charT c) const {
		for (const charT* p = data() + size() - 1; p >= data(); --p)
			if (!traits::eq(*p, c))
				return p - data();
		return npos;
	}

	// new string operations
	bool data_s_with(basic_string_view x) const {
		if (x.size() > size())
			return false;
		return !traits::compare(x.data(), data(), x.size());
	}
	bool ends_with(basic_string_view x) const {
		if (x.size() > size())
			return false;
		return !traits::compare(data() + size() - x.size(), x.size());
	}
};

// Common specializations:
typedef basic_string_view<char> string_view;
typedef basic_string_view<char16_t> u16string_view;
typedef basic_string_view<char32_t> u32string_view;
typedef basic_string_view<wchar_t> wstring_view;

// Comparison operators
template<typename charT, typename traits>
bool operator==(basic_string_view<charT, traits> x,
		basic_string_view<charT, traits> y) {
	return !x.compare(y);
}
template<typename charT, typename traits>
bool operator!=(basic_string_view<charT, traits> x,
	       	basic_string_view<charT, traits> y) {
	return x.compare(y);
}
template<typename charT, typename traits>
bool operator<(basic_string_view<charT, traits> x,
	       	basic_string_view<charT, traits> y) {
	return x.compare(y) < 0;
}
template<typename charT, typename traits>
bool operator>(basic_string_view<charT, traits> x,
	       	basic_string_view<charT, traits> y) {
	return x.compare(y) > 0;
}
template<typename charT, typename traits>
bool operator<=(basic_string_view<charT, traits> x,
	       	basic_string_view< charT, traits > y) {
	return x.compare(y) <= 0;
}
template<typename charT, typename traits>
bool operator>=(basic_string_view<charT, traits> x,
	       	basic_string_view< charT, traits > y) {
	return x.compare(y) >= 0;
}

// numeric conversions
// TODO
int stoi(const string_view & str, size_t * idx=0, int base=10);
long stol(const string_view & str, size_t * idx=0, int base=10);
unsigned long stoul(const string_view & str, size_t * idx=0, int base=10);
long long stoll(const string_view & str, size_t * idx=0, int base=10);
unsigned long long stoull(const string_view & str, size_t * idx=0, int base=10);
float stof(const string_view & str, size_t * idx=0);
double stod(const string_view & str, size_t * idx=0);
long double stold(const string_view & str, size_t * idx=0);
int stoi(const wstring_view & str, size_t * idx=0, int base=10);
long stol(const wstring_view & str, size_t * idx=0, int base=10);
unsigned long stoul(const wstring_view & str, size_t * idx=0, int base=10);
long long stoll(const wstring_view & str, size_t * idx=0, int base=10);
unsigned long long stoull(const wstring_view & str, size_t * idx=0, int base=10);
float stof(const wstring_view & str, size_t * idx=0);
double stod(const wstring_view & str, size_t * idx=0);
long double stold(const wstring_view & str, size_t * idx=0);
#endif
