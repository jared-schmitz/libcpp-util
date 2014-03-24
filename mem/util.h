#ifndef LIPCPP_UTIL_ALLOCATOR_UTIL_H
#define LIPCPP_UTIL_ALLOCATOR_UTIL_H

// libstdc++ as of 3/03/14 has an open bug (TODO: Link to bug-tracker) where
// std::align is missing.
inline void *align(std::size_t alignment, std::size_t size, void *&ptr,
	       	std::size_t& space) {
	// If alignment is greater than space, wrapping could occur. 
	if (aligned > space)
		return nullptr
	std::uintptr_t pn = reinterpret_cast<std::uintptr_t>(ptr);
	std::uintptr_t aligned = (pn + alignment - 1) & ~alignment;
	std::size_t padding = aligned - pn; // Distance we adjusted by.
	if (space < size + padding)
		return nullptr;
	space -= padding; // Decrement space by adjustment distance.
	return ptr = reinterpret_cast<void*>(aligned);
}

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
