#ifndef LIBCPP_UTIL_RING_BUFFER_H
#define LIBCPP_UTIL_RING_BUFFER_H

#include "libcpp/util/raw_array.h"
#include <memory>
#include <cstddef>

template <typename T, size_t N, class Alloc = std::allocator<T>>
class ring_buffer {
private:
	static std::size_t increment_index(std::size_t& index, unsigned N) {
		std::size_t tmp = index;
		index = (index + 1) % N;
		return tmp;
	}

	size_t head, tail;

	circ_fifo(const circ_fifo&) = delete;
	circ_fifo& operator=(const circ_fifo&) = delete;

	Alloc alloc;
	raw_array<T, N> fifo;

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

	bool empty() const {
		return head == tail;
	}

	template <class... Args>
	void emplace(Args&&... args) {
		auto tmp_tail = increment_index(tail, N);
		alloc.construct(&fifo[tmp_tail], std::forward<Args>(args)...);
	}
	void push(const T& val) {
		auto tmp_tail = increment_index(tail, N);
		alloc.construct(&fifo[tmp_tail], val);
	}

	void push(T&& val) {
		auto tmp_tail = increment_index(tail, N);
		alloc.construct(&fifo[tmp_tail], std::move(val));
	}

	bool pop_value(T& val) {
		if (head == tail)
			return false;
		auto tmp_head = increment_index(head, N);
		val = std::move(fifo[tmp_head]);
		alloc.destroy(&fifo[tmp_head]);
		return true;
	}
};

#endif
