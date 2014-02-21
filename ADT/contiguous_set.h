#ifndef LIBCPP_CONTIGUOUS_SET_H
#define LIBCPP_CONTIGUOUS_SET_H

template <class Key, class Compare = std::less<Key>,
	  class Allocator = std::allocator<Key>>
class contiguous_set : public sorted_vector<Key, Compare, Allocator> {}

#endif
