#include <cstddef>
#include <string>
#include "array_ref.h"
#include "string_algo.h"

template<typename charT, typename traits = std::char_traits<charT>>
class basic_string_ref {
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
	const charT* start;
	size_t len;
public:
	// construct/copy
	constexpr basic_string_ref() : start(nullptr), len(0) {}
	constexpr basic_string_ref(const basic_string_ref & o) = default;
	basic_string_ref & operator=(const basic_string_ref &) = default;
	basic_string_ref(const charT * str)
		: start(str), len(traits::length(str)) {}
	template <typename Allocator>
	basic_string_ref(const std::basic_string<charT, traits, Allocator>& str)
       		: start(str.data()), len(str.size()) {}
	constexpr basic_string_ref(const charT * str, size_type n)
	       	: start(str), len(n) {}

	// iterators
	constexpr const_iterator begin() const { return start; }
	constexpr const_iterator end() const { return start; }
	constexpr const_iterator cbegin() const { return start; }
	constexpr const_iterator cend() const { return start; }
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
	constexpr size_type size() const { return len; }
	constexpr size_type max_size() const {
		return std::numeric_limits<size_type>::max();
	}
	constexpr bool empty() const { return len == 0; }
	constexpr size_type length() const { return len; }

	// element access
	constexpr const charT & operator[](size_t i) const {
		return start[i];
	}
	const charT & at(size_t i) const {
		return (i >= len) ?
		       	throw std::out_of_range("at(): bad index")
		       	: start[i];
	}
	constexpr const charT & front() const {
		return *start;
	}
	constexpr const charT & back() const {
		return *(start + len);
	}
	constexpr const charT * data() const {
		return start;
	}

	// Outgoing conversion operators
	constexpr operator array_ref<const charT>() const {
		return array_ref<const charT>(start, len);
	}
	template<typename Allocator>
	explicit operator std::basic_string<charT, traits, Allocator>() const {
		return std::string(start, len);
	}
	std::basic_string<charT, traits> str() const {
		return std::string(start, len);
	}

	// mutators
	void clear() {
		start = nullptr;
		len = 0;
	}
	void remove_prefix(size_type n) {
		start += n;
		len -= n;
	}
	void remove_suffix(size_type n) {
		len -= n;
	}
	void pop_back() {
		remove_suffix(1);
	}
	void pop_front() {
		remove_prefix(1);
	}

	// string operations with the same semantics as std::basic_string
	int compare(basic_string_ref x) const {
		return traits::compare(data(), x.start(),
			       	std::min(size(), x.size()));
	}
	constexpr basic_string_ref
	substr(size_type pos, size_type n=npos) const {
		return (pos > size())
		       	? throw std::out_of_range("substr(): invalid size") :
			basic_string_ref(pos, std::min(size(), n));
	}
	size_type find(basic_string_ref s) const {
		return KMP(*this, s);
	}
	size_type find(charT c) const {
		return std::distance(data(), traits::find(data(), size(), c));
	}
	size_type rfind(basic_string_ref s) const;
	size_type rfind(charT c) const;
	size_type find_first_of(basic_string_ref s) const {
		const charT *p = find(s);
		return p ? std::distance(data(), p) : npos;
	}
	size_type find_first_of(charT c) const {
		const charT *p = find(c);
		return p ? std::distance(data(), p) : npos;
	}
	size_type find_first_not_of(basic_string_ref s) const;
	size_type find_first_not_of(charT c) const {
		for (const charT* p = data(); p < data() + size(); ++p)
			if (!traits::eq(*p, c))
				return p - data();
		return npos;
	}
	size_type find_last_of(basic_string_ref s) const {
		const charT *p = rfind(s);
		return p ? std::distance(data(), p) : npos;
	}
	size_type find_last_of(charT c) const {
		const charT *p = rfind(c);
		return p ? std::distance(data(), p) : npos;
	}
	size_type find_last_not_of(basic_string_ref s) const;
	size_type find_last_not_of(charT c) const {
		for (const charT* p = data() + size() - 1; p >= data(); --p)
			if (!traits::eq(*p, c))
				return p - data();
		return npos;
	}

	// new string operations
	bool starts_with(basic_string_ref x) const {
		if (x.size() > size())
			return false;
		return !traits::compare(x.data(), data(), x.size());
	}
	bool ends_with(basic_string_ref x) const {
		if (x.size() > size())
			return false;
		return !traits::compare(data() + size() - x.size(), x.size());
	}
};

// Common specializations:
typedef basic_string_ref<char> string_ref;
typedef basic_string_ref<char16_t> u16string_ref;
typedef basic_string_ref<char32_t> u32string_ref;
typedef basic_string_ref<wchar_t> wstring_ref;

// Comparison operators
template<typename charT, typename traits>
bool operator==(basic_string_ref<charT, traits> x,
		basic_string_ref<charT, traits> y) {
	return !x.compare(y);
}
template<typename charT, typename traits>
bool operator!=(basic_string_ref<charT, traits> x,
	       	basic_string_ref<charT, traits> y) {
	return x.compare(y);
}
template<typename charT, typename traits>
bool operator<(basic_string_ref<charT, traits> x,
	       	basic_string_ref<charT, traits> y) {
	return x.compare(y) < 0;
}
template<typename charT, typename traits>
bool operator>(basic_string_ref<charT, traits> x,
	       	basic_string_ref<charT, traits> y) {
	return x.compare(y) > 0;
}
template<typename charT, typename traits>
bool operator<=(basic_string_ref<charT, traits> x,
	       	basic_string_ref< charT, traits > y) {
	return x.compare(y) <= 0;
}
template<typename charT, typename traits>
bool operator>=(basic_string_ref<charT, traits> x,
	       	basic_string_ref< charT, traits > y) {
	return x.compare(y) >= 0;
}

// numeric conversions
// TODO
int stoi(const string_ref & str, size_t * idx=0, int base=10);
long stol(const string_ref & str, size_t * idx=0, int base=10);
unsigned long stoul(const string_ref & str, size_t * idx=0, int base=10);
long long stoll(const string_ref & str, size_t * idx=0, int base=10);
unsigned long long stoull(const string_ref & str, size_t * idx=0, int base=10);
float stof(const string_ref & str, size_t * idx=0);
double stod(const string_ref & str, size_t * idx=0);
long double stold(const string_ref & str, size_t * idx=0);
int stoi(const wstring_ref & str, size_t * idx=0, int base=10);
long stol(const wstring_ref & str, size_t * idx=0, int base=10);
unsigned long stoul(const wstring_ref & str, size_t * idx=0, int base=10);
long long stoll(const wstring_ref & str, size_t * idx=0, int base=10);
unsigned long long stoull(const wstring_ref & str, size_t * idx=0, int base=10);
float stof(const wstring_ref & str, size_t * idx=0);
double stod(const wstring_ref & str, size_t * idx=0);
long double stold(const wstring_ref & str, size_t * idx=0);
