//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_SPSC_CIRC_FIFO_H
#define LIBCPP_UTIL_SPSC_CIRC_FIFO_H

#include "libcpp-util/util/raw_array.h"

#include <atomic>
#include <memory>
#include <cassert>
#include <cstddef>
#include <type_traits>

namespace cpputil {

template <bool MultiProducer, bool MultiConsumer>
struct atomic_discipline {
private:
	using nonatomic = std::size_t;
	using atomic = std::atomic_size_t;

	template <bool Atomic>
	using atomic_t = typename std::conditional<Atomic, atomic,
					     std::size_t>::type;

public:
	using head_index_type = atomic_t<MultiConsumer>;
	using tail_index_type = atomic_t<MultiProducer>;

	static std::size_t increment_index(nonatomic& index, unsigned N) {
		std::size_t tmp = index;
		index = (index + 1) % N;
		return tmp;
	}

};

// FIXME: This implementation is exception-unsafe. When an exception is thrown
// while copying an object, a block of undefined bits is left there to be
// consumed later by a pull operation. Instead check the relevant functions for
// noexcept and don't release the lock until we have completed putting the item
// in the queue. If an exception is thrown, decrement the index and rethrow.
template <typename T, size_t N, class Alloc = std::allocator<T>>
class spsc_nb_queue {
private:
	std::atomic_bool open;

	std::atomic_size_t full, empty, head, tail;

	concurrent_queue(const concurrent_queue&) = delete;
	concurrent_queue& operator=(const concurrent_queue&) = delete;
	concurrent_queue(concurrent_queue&&) = delete;
	concurrent_queue& operator=(concurrent_queue&&) = delete;

	Alloc alloc;
	raw_array<T, N> fifo;

	std::size_t increment_index(atomic& index) {
		std::size_t old_count, new_count;
		do {
			old_count = index;
			new_count = (old_count + 1) % N;
		} while (!index.compare_exchange_strong(old_count, new_count));
		return old_count;
	}

	bool pop_value_common(T& val) {
		auto tmp_head = AtomicPolicy::increment_index(head, N);
		full--;
		val = std::move(fifo[tmp_head]);
		alloc.destroy(&fifo[tmp_head]);
		empty++;
		return true;
	}

	bool queue_is_finished() {
		return is_closed() && full == 0;
	}

public:
	concurrent_queue() : open(true), full(0), empty(N), head(0), tail(0) {
	}
	// TODO: Other standard library type constructors
	~concurrent_queue() {
		while (head != tail) {
			alloc.destroy(&fifo[head]);
			head = (head + 1) % N;
		}
	}

	bool is_closed() const {
		return !open;
	}

	void close() {
		open = false;
	}

	constexpr size_t capacity() const {
		return N;
	}

	bool try_push(const T& val) {
		// If empty is 0, no empty slots; nowhere to push data. Return false.
		if (!empty)
			return false;
		// Grab the empty spot.
		empty--;
		auto tmp_tail = AtomicPolicy::increment_index(tail, N);
		l.unlock();
		alloc.construct(&fifo[tmp_tail], val);
		full++;
	}

	bool try_pop(T& val) {
		if (!full)
			return false;
		if (queue_is_finished())
			return false;
		return pop_value_common(val);
	}
};

} // End namespace
#endif
