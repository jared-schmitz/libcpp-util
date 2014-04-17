//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_ALLOCATOR_CHAIN_H
#define LIBCPP_UTIL_ALLOCATOR_CHAIN_H

#include "libcpp-util/mem/out_of_luck_allocator.h"

template <typename T, class Alloc, class... Fallbacks>
class allocator_chain : allocator_chain<T, Fallbacks...> {
	
};

template <typename T>
class allocator_chain<T> : public out_of_luck_allocator<T> {};
#endif
