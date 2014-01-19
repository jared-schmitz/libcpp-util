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
	std::atomic_bool open;
	std::atomic<size_t> writers, readers;
public:
	atomic_discipline() : open(true), writers(0), readers(0) {}

	typedef std::size_t nonatomic_type;
	typedef std::atomic_size_t atomic_type;

	typedef semaphore sem_type;
	typedef typename std::conditional<MultiConsumer, atomic_type,
	       	std::size_t>::type head_index_type;
	typedef typename std::conditional<MultiProducer, atomic_type,
		std::size_t>::type tail_index_type;

	typedef ref_count_handle<std::size_t> write_ticket;
	typedef ref_count_handle<std::size_t> read_ticket;

	write_ticket write_open() {
		open = false;
		return write_ticket(writers);
	}
	read_ticket read_open() {
		return read_ticket(readers);
	}
	bool is_closed() const {
		return !open && writers.load() == 0;
	}

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

struct nonatomic_discipline {
	struct noop_semaphore {
		void post() {}
		void wait() {}
		unsigned value() const { return 0; }
		noop_semaphore(unsigned) {}
		~noop_semaphore() = default;
	};
	void close() {}
	bool is_closed() const { return false; }

	typedef std::size_t nonatomic_type;
	typedef std::size_t atomic_type;

	typedef noop_semaphore sem_type;
	typedef std::size_t head_index_type;
	typedef std::size_t tail_index_type;

	static std::size_t increment_index(std::size_t& index, unsigned N) {
		std::size_t tmp = index;
		index = (index + 1) % N;
		return tmp;
	}
};

template <typename T, size_t N, typename AtomicPolicy>
class circ_fifo : public AtomicPolicy {
private:
	typename AtomicPolicy::sem_type full, empty;
	typename AtomicPolicy::head_index_type head;
	typename AtomicPolicy::tail_index_type tail;

	using AtomicPolicy::is_closed;

	circ_fifo(const circ_fifo&) = delete;
	circ_fifo& operator=(const circ_fifo&) = delete;

	std::allocator<T> alloc;
	raw_array<T, N> fifo;

	bool pop_value_common(T& val) {
		auto tmp_head = AtomicPolicy::increment_index(head, N);
		val = std::move(fifo[tmp_head]);
		alloc.destroy(&fifo[tmp_head]);
		empty.post();
		return true;
	}

	bool queue_is_finished() const {
		return is_closed() && full.value() == 0;
	}
public:
	circ_fifo() : full(0), empty(N), head(0), tail(0) {}
	// TODO: Other standard library type constructors
	~circ_fifo() {
		while (head != tail) {
			alloc.destroy(&fifo[head]);
			head = (head + 1) % N;
		}
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

	bool pop_value(T& val) {
		if (queue_is_finished())
			return false;
		full.wait();
		return pop_value_common(val);
	}

	bool try_pop_value(T& val) {
		if (queue_is_finished())
			return false;
		if (!full.try_wait())
			return false;
		return pop_value_common(val);
	}
};

template <typename T, unsigned N>
using nonatomic_circ_fifo = circ_fifo<T, N, nonatomic_discipline>;
template <typename T, unsigned N>
using spsc_circ_fifo = circ_fifo<T, N, spsc_discipline>;
template <typename T, unsigned N>
using spmc_circ_fifo = circ_fifo<T, N, spmc_discipline>;
template <typename T, unsigned N>
using mpsc_circ_fifo = circ_fifo<T, N, mpsc_discipline>;
template <typename T, unsigned N>
using mpmc_circ_fifo = circ_fifo<T, N, mpmc_discipline>;

} // End namespace
#endif
