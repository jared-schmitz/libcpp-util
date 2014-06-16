//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIPCPP_UTIL_ALLOCATOR_UTIL_H
#define LIPCPP_UTIL_ALLOCATOR_UTIL_H

#include <cstdint>
#include <cstddef>
#include <utility>

#if defined(_WIN32)
// aligned_alloc will eventually be supported when C11 is.
#define aligned_alloc(align, size) _aligned_malloc(size, align)
#endif

// libstdc++ as of 3/03/14 has an open bug (TODO: Link to bug-tracker) where
// std::align is missing.
inline void *align(std::size_t alignment, std::size_t size, void *&ptr,
	       	std::size_t& space) {
	// If alignment is greater than space, wrapping could occur. 
	if (alignment > space)
		return nullptr;
	std::uintptr_t pn = reinterpret_cast<std::uintptr_t>(ptr);
	std::uintptr_t aligned = (pn + alignment - 1) & ~alignment;
	std::size_t padding = aligned - pn; // Distance we adjusted by.
	if (space < size + padding)
		return nullptr;
	space -= padding; // Decrement space by adjustment distance.
	return ptr = reinterpret_cast<void*>(aligned);
}

// Depending on the compilation environment or user preference, do different
// things when we can't fulfill an allocation. The option dictated by the
// standard is to throw std::bad_alloc. We might also want to return a nullptr.
struct alloc_exception_policy {
	void on_allocate_failure(std::size_t) {
		throw std::bad_alloc();
	}
	void deallocate_fallback(void*, std::size_t) {}
};

struct alloc_nullptr_policy {
	void *on_allocate_failure(std::size_t) {
		return nullptr;
	}
	void deallocate_fallback(void*, std::size_t) {}
};

// Useful for tuning allocators
struct stats_policy {
private:
	size_t total_bytes;
	size_t total_elems;
	size_t total_ctor;
	size_t total_dtor;

	size_t highwater_bytes;
public:

	void account_alloc(size_t nr_bytes, size_t nr_elems) {
	}
	void account_dealloc(size_t nr_bytes, size_t nr_elems) {
	}

	void account_construct() {
		total_ctor++;
	}
	void account_destroy() {
		total_dtor++;
	}
};

// As we many implementations don't have fully allocator-aware container
// implementations by C++11 definition, since they were added relatively late,
// we use this to reduce some of the boilerplate. This class exposes the
// definitions that std::allocator_traits would fill in. It is slightly less
// convenient, because it doesn't (can't?) provide some of the default
// implementations that are provided through the traits class. It also allows
// conversions that would we not prefer, so we make the destructor protected to
// prevent slicing off the entire real allocator.
template <typename T>
class no_cxx11_allocators {
protected:
	~no_cxx11_allocators() = default; // Prevent accidental deletion
public:
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	
	template <typename U, typename... Args>
	void construct(U* p, Args&&... args) {
		::new((void*)p) U(std::forward<Args>(args)...);
	}

	template <typename U>
	void destroy(U* p) {
		p->~U();
	}
};

#endif
