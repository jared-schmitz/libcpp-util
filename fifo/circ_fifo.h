#ifndef LIBCPP_SMP_CIRC_FIFO_H
#define LIBCPP_SMP_CIRC_FIFO_H

#include "libcpp-smp/util/raw_array.h"

#include <cstddef>
#include <memory>

namespace libcpp-smp {

class circ_fifo_base {
protected:
	size_t head, tail; // Tail is one past
	size_t _size;
public:
	circ_fifo_base() : head(0), tail(0), _size(0) {}
	circ_fifo_base(const circ_fifo_base& o) :
		head(o.head), tail(o.tail), _size(o._size) {}
	circ_fifo_base& operator=(const circ_fifo_base& o) {
		head = o.head;
		tail = o.tail;
		_size = o._size;
	}
	size_t size() const { return _size; }
	bool empty() const { return _size == 0; }
};

// An old-fashioned ringbuffer. Doesn't support iteration, only allows head
// and tail access. As a bonus, it acts as a doubly-ended queue instead of
// just a FIFO.
template <typename T, size_t N>
class circ_fifo : public circ_fifo_base {
private:
	std::allocator<T> alloc;
	raw_array<T, N> data;

	size_t prev(size_t pos) const {
		if (pos == 0)
			return N - 1;
		else
			return pos - 1;
	}
	size_t next(size_t pos) const {
		return (pos + 1) % N;
	}
	void increment(size_t& pos) {
		pos = next(pos);
	}
	void decrement(size_t& pos) {
		pos = prev(pos);
	}

	// Copies while linearizing
	void copy_to_array(std::array<T, N>& dest) {
		size_t out_pos = 0, in_pos = head;
		for ( ; out_pos < _size; ++out_pos, increment(in_pos))
			dest[out_pos] = data[in_pos];
	}
	using circ_fifo_base::head;
	using circ_fifo_base::tail;
	using circ_fifo_base::_size;
public:
	typedef T value_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef T* pointer;
	typedef const T* const_pointer;

	circ_fifo() = default;
	circ_fifo(const circ_fifo& o) : alloc(o.alloc) {
		o.copy_to_array(data);
	}
	circ_fifo& operator=(const circ_fifo& o) {
		alloc = o.alloc;
		o.copy_to_array(data);
	}
	~circ_fifo() {
		while (head != tail) {
			alloc.destroy(&data[head]);
			increment(head);
		}
	}

	constexpr size_t capacity() const { return data.capacity(); }
	bool full() const { return size() == data.capacity(); }

	void push_back(const T& val) {
		if (full()) {
			data[tail] = val;
			increment(tail);
			head = tail;
		} else {
			alloc.construct(&data[tail], val);
			increment(tail);
			++_size;
		}
	}
	void push_front(const T& val) {
		if (full()) {
			decrement(head);
			data[head] = val;
			tail = head;
		} else {
			decrement(head);
			alloc.construct(&data[head], val);
			++_size;
		}
	}

	void pop_front() {
		alloc.destroy(head);
		increment(head);
		--_size;
	}
	void pop_back() {
		decrement(tail);
		alloc.destroy(tail);
		--_size;
	}

	const T& front() const {
		return data[head];
	}
	T& front() {
		return data[head];
	}

	const T& back() const {
		return data[prev(tail)];
	}
	T& back() {
		return data[prev(tail)];
	}

	void clear() {
		while (head != tail) {
			alloc.destroy(&data[head]);
			increment(head);
		}
		_size = 0;
		head = 0;
		tail = 0;
       	}

	void swap(circ_fifo& other) {
		circ_fifo tmp = other;
		other = *this;
		*this = tmp;
	}

	bool is_linear() const {
		if (head != 0)
			return false;
	}

	void make_linear() {
		// FIXME: This could be made much faster
		circ_fifo tmp(*this);
		swap(tmp);
	}
};

} // End namespace
#endif
