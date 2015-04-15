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
	typedef std::size_t nonatomic_type;
	typedef std::atomic_size_t atomic_type;

	typedef semaphore sem_type;
	typedef typename std::conditional<MultiConsumer, atomic_type,
	       	std::size_t>::type head_index_type;
	typedef typename std::conditional<MultiProducer, atomic_type,
		std::size_t>::type tail_index_type;

	static std::size_t increment_index(std::size_t& index, unsigned N) {
		std::size_t tmp = index;
		index = (index + 1) % N;
		return tmp;
	}

	static std::size_t increment_index(atomic_type& index, unsigned N) {
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

template <typename T, size_t N, class AtomicPolicy,
	 class Alloc = std::allocator<T>>
class concurrent_queue : public AtomicPolicy {
private:
	std::atomic_bool open;

	typename AtomicPolicy::sem_type full, empty;
	typename AtomicPolicy::head_index_type head;
	typename AtomicPolicy::tail_index_type tail;

	concurrent_queue(const concurrent_queue&) = delete;
	concurrent_queue& operator=(const concurrent_queue&) = delete;

	Alloc alloc;
	raw_array<T, N> fifo;

	bool pop_value_common(T& val) {
		auto tmp_head = AtomicPolicy::increment_index(head, N);
		val = std::move(fifo[tmp_head]);
		alloc.destroy(&fifo[tmp_head]);
		empty.post();
		return true;
	}

	bool queue_is_finished() {
		return is_closed() && full.value() == 0;
	}
public:
	concurrent_queue() : open(true), full(0), empty(N), head(0), tail(0) {}
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
		full.post_all(); // Make sure to wake any readers
	}

	constexpr size_t capacity() const {
		return N;
	}

	template <class... Args>
	void emplace(Args&&... args) {
		empty.wait();
		auto tmp_tail = AtomicPolicy::increment_index(tail, N);
		alloc.construct(&fifo[tmp_tail], std::forward<Args>(args)...);
		full.post();
	}
	void push(const T& val) {
		empty.wait();
		auto tmp_tail = AtomicPolicy::increment_index(tail, N);
		alloc.construct(&fifo[tmp_tail], val);
		full.post();
	}

	void push(T&& val) {
		empty.wait();
		auto tmp_tail = AtomicPolicy::increment_index(tail, N);
		alloc.construct(&fifo[tmp_tail], std::move(val));
		full.post();
	}

	bool pop(T& val) {
		full.wait();
		if (queue_is_finished())
			return false;
		return pop_value_common(val);
	}

	bool try_pop(T& val) {
		if (!full.try_wait())
			return false;
		if (queue_is_finished())
			return false;
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
