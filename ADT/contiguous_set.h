//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_CONTIGUOUS_SET_H
#define LIBCPP_CONTIGUOUS_SET_H

#include "libcpp-util/ADT/sorted_vector.h"

template <class Key, class Compare = std::less<Key>,
	  class Allocator = std::allocator<Key>>
class contiguous_set : public sorted_vector<Key, Compare, Allocator> {};

#endif
