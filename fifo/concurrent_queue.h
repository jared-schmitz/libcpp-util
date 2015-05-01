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
#include "libcpp-util/util/ref_count_handle.h"

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

	static std::size_t increment_index(atomic& index, unsigned N) {
		std::size_t old_count, new_count;
		do {
			old_count = index;
			new_count = (old_count + 1) % N;
		} while (!index.compare_exchange_strong(old_count, new_count));
		return old_count;
	}
};
typedef atomic_discipline<false, false> spsc_discipline;
typedef atomic_discipline<true, false> mpsc_discipline;
typedef atomic_discipline<false, true> spmc_discipline;
typedef atomic_discipline<true, true> mpmc_discipline;

// FIXME: This implementation is exception-unsafe. When an exception is thrown
// while copying an object, a block of undefined bits is left there to be
// consumed later by a pull operation. Instead check the relevant functions for
// noexcept and don't release the lock until we have completed putting the item
// in the queue. If an exception is thrown, decrement the index and rethrow.
template <typename T, size_t N, class AtomicPolicy,
	  class Alloc = std::allocator<T>>
class concurrent_queue : public AtomicPolicy {
private:
	std::atomic_bool open;

	std::condition_variable empty_cv, full_cv;
	std::mutex lock;

	std::size_t full, empty;
	typename AtomicPolicy::head_index_type head;
	typename AtomicPolicy::tail_index_type tail;

	concurrent_queue(const concurrent_queue&) = delete;
	concurrent_queue& operator=(const concurrent_queue&) = delete;
	concurrent_queue(concurrent_queue&&) = delete;
	concurrent_queue& operator=(concurrent_queue&&) = delete;

	Alloc alloc;
	raw_array<T, N> fifo;

	enum class wait_result {
		ready,
		closed
	};

	wait_result wait_for_used_space_or_close() {
		std::unique_lock<std::mutex> l(lock);
		while (1) {
			if (full)
				return wait_result::ready;
			else if (!open)
				return wait_result::closed;
			full_cv.wait(l);
		}
	}

	void wait_for_empty_space() {
		std::unique_lock<std::mutex> l(lock);
		while (1) {
			if (empty)
				return;
			empty_cv.wait(l);
		}
	}

	bool pop_value_common(T& val) {
		auto tmp_head = AtomicPolicy::increment_index(head, N);
		full--;
		val = std::move(fifo[tmp_head]);
		alloc.destroy(&fifo[tmp_head]);
		empty++;
		empty_cv.notify_one();
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
		// XXX: We check if already closed because we don't want to signal
		// everyone again as we're destroying things.
		if (is_closed())
			close();
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
		full_cv.notify_all(); // Make sure to wake any readers
	}

	constexpr size_t capacity() const {
		return N;
	}

	template <class... Args>
	void emplace(Args&&... args) {
		wait_for_empty_space();
		empty--;
		auto tmp_tail = AtomicPolicy::increment_index(tail, N);
		alloc.construct(&fifo[tmp_tail], std::forward<Args>(args)...);
		full++;
		full_cv.notify_one();
	}

	void push(const T& val) {
		wait_for_empty_space();
		empty--;
		auto tmp_tail = AtomicPolicy::increment_index(tail, N);
		alloc.construct(&fifo[tmp_tail], val);
		full++;
		full_cv.notify_one();
	}

	void push(T&& val) {
		wait_for_empty_space();
		empty--;
		auto tmp_tail = AtomicPolicy::increment_index(tail, N);
		alloc.construct(&fifo[tmp_tail], std::move(val));
		full++;
		full_cv.notify_one();
	}

	bool try_push(const T& val) {
		std::unique_lock<std::mutex> l(lock);
		if (!empty)
			return false;
		empty--;
		auto tmp_tail = AtomicPolicy::increment_index(tail, N);
		l.unlock();
		alloc.construct(&fifo[tmp_tail], val);
		full++;
		full_cv.notify_one();
	}

	bool pop(T& val) {
		switch (wait_for_used_space_or_close()) {
		case wait_result::ready:
			return pop_value_common(val);
		case wait_result::closed:
			return false;
		}
		return false; // Shut up compiler.
	}

	bool try_pop(T& val) {
		std::unique_lock<std::mutex> l(lock);
		if (!full)
			return false;
		if (queue_is_finished())
			return false;
		l.unlock();
		return pop_value_common(val);
	}
};

template <typename T, unsigned N>
using spsc_queue = concurrent_queue<T, N, spsc_discipline>;
template <typename T, unsigned N>
using spmc_queue = concurrent_queue<T, N, spmc_discipline>;
template <typename T, unsigned N>
using mpsc_queue = concurrent_queue<T, N, mpsc_discipline>;
template <typename T, unsigned N>
using mpmc_queue = concurrent_queue<T, N, mpmc_discipline>;

} // End namespace
#endif
