//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_SPSC_CIRC_FIFO_H
#define LIBCPP_UTIL_SPSC_CIRC_FIFO_H

#include "libcpp-util/smp/semaphore.h"
#include "libcpp-util/util/raw_array.h"

#include <atomic>
#include <memory>
#include <cassert>
#include <cstddef>
#include <type_traits>

namespace cpputil {

class spsc_circ_fifo_base {
protected:
	std::atomic<bool> open;
	semaphore full, empty;
	size_t head, tail;
public:
	spsc_circ_fifo_base(size_t N)
		: open(true), full(0), empty(N), head(0), tail(0) {}

};

template <typename T, size_t N>
class spsc_circ_fifo : public spsc_circ_fifo_base {
private:

	spsc_circ_fifo(const spsc_circ_fifo&) = delete;
	spsc_circ_fifo& operator=(const spsc_circ_fifo&) = delete;

	std::allocator<T> alloc;
	raw_array<T, N> fifo;

	bool pop_value_common(T& val) {
		if (queue_is_finished())
			return false;
		val = std::move(fifo[head]);
		alloc.destroy(&fifo[head]);
		head = (head + 1) % N;
		empty.post();
		return true;
	}

	bool queue_is_finished() const {
		return closed() && full.value() == 0;
	}
	using spsc_circ_fifo_base::head;
	using spsc_circ_fifo_base::tail;
	using spsc_circ_fifo_base::open;
	using spsc_circ_fifo_base::full;
	using spsc_circ_fifo_base::empty;
public:
	spsc_circ_fifo() : spsc_circ_fifo_base(N) {}
	// TODO: Other standard library type constructors
	~spsc_circ_fifo() {
		while (head != tail) {
			alloc.destroy(&fifo[head]);
			head = (head + 1) % N;
		}
	}

	constexpr size_t capacity() const {
		return N;
	}

	bool closed() const {
		return !open;
	}

	// Signal that we are done
	void close() {
		assert(open && "Cannot close closed fifo");
		open = false;
		full.post(); // So the reader will wake up and exit
	}

	template <class... Args>
	void emplace(Args&&... args) {
		empty.wait();
		assert(open && "Pushing to closed fifo");
		alloc.construct(&fifo[tail], std::forward<Args>(args)...);
		tail = (tail + 1) % N;
		full.post();
	}
	void push(const T& val) {
		empty.wait();
		assert(open && "Pushing to closed fifo");
		alloc.construct(&fifo[tail], val);
		tail = (tail + 1) % N;
		full.post();
	}

	void push(T&& val) {
		empty.wait();
		assert(open && "Pushing to closed fifo");
		alloc.construct(&fifo[tail], std::move(val));
		tail = (tail + 1) % N;
		full.post();
	}

	bool pop_value(T& val) {
		full.wait();
		return pop_value_common(val);
	}

	bool try_pop_value(T& val) {
		if (!full.try_wait())
			return false;
		return pop_value_common(val);
	}
};

} // End namespace

#endif
