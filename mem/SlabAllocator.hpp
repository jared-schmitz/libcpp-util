#include <cassert>
#include <cstddef>
#include <cstdint>
#include <list>
#include <set>
#include <array>
#include <cstring>

template <typename T>
class slab_allocator_base {
private:
	class slab;
	std::list<slab*> slabs_free;
	std::list<slab*> slabs_partial;
	std::list<slab*> slabs_full;
	slab* hot_slab; // Last slab used and not full. If NULL, search

	// Singleton
	slab_allocator_base() : hot_slab(0) {
		for (unsigned i = 0; i < 4; ++i)
			get_new_slab();

	}
	~slab_allocator_base() {
		for (const auto& slabs : slabs_free)
			delete slabs;
#ifndef NDEBUG
		slabs_free.clear();
		assert(slabs_partial.empty() && slabs_full.empty() && "Memory leak");
#endif
	}

	// For objects less than 1/8 of the page size, the slab should contain
	// the maximum amount of objects that fit into the page, minus the
	// space for the freemap.
	// For objects more than 1/8 of the page size, the slab should be a
	// reasonable multiple of the page size (2, 4, 8) and some smarter way
	// to figure out what buffers are free.
	// FIXME: This will break with objects that are bigger than a page
	class slab {
		int16_t next_free_cache;
		uint16_t size;
		std::array<bool, 4096 / (sizeof(T) + 1)> _free;
		// FIXME: Align this guy
		unsigned char slab_data[(4096 / (sizeof(T) + 1)) * sizeof(T)];
		size_t next_free() {
			if (next_free_cache == -1) {
				bool *r = (bool*)memchr(&_free[0], true,
						_free.size());
				assert(r && "None free, why not reallocate?");
				return r - &_free[0];
			}
			// If the next one is free, cache it
			int16_t old_next = next_free_cache;
			int16_t next = (next_free_cache + 1) % _free.size();
			if (next == true)
				next_free_cache = next;
			else
				next_free_cache = -1;
			return old_next;
		}
	public:
		slab() : next_free_cache(0), size(0) {
			_free.fill(true);
		}
		typename std::list<slab*>::iterator link;
		bool full() const {
			return size == _free.size();
		}
		bool free() const {
			return size == 0;
		}
		T* get() {
			size_t position = next_free();
			_free[position] = false;
			++size;
			return reinterpret_cast<T*>(&slab_data[position * sizeof(T)]);
		}
		void put(const T* p) {
			_free[p - ((T*)&slab_data[0])] = true;
			--size;
		}
		struct slab_sorter {
			bool operator()(const slab* lhs,
				       	const slab* rhs) const {
				return &lhs->slab_data[0] < &rhs->slab_data[0];
			}
		};
	};

	// Used to map a pointer to be destroyed back to slab. Free slabs do
	// not live in here.
	std::set<slab*, typename slab::slab_sorter> slabs_sort;

	slab* get_new_slab() {
		slab* new_slab = new slab();
		slabs_free.push_front(new_slab);
		new_slab->link = slabs_free.begin();
		return new_slab;
	}
	slab* get_best_slab() {
		if (!hot_slab) {
			if (!slabs_partial.empty())
				hot_slab = slabs_partial.front();
			else if (!slabs_free.empty())
				hot_slab = slabs_free.front();
			else
				hot_slab = get_new_slab();
		}
		return hot_slab;
	}
	slab* find_slab(T* p) {
		auto i = slabs_sort.lower_bound((slab*)p);
		if (i == slabs_sort.end()) {
			return *slabs_sort.rbegin();
		}
		return *std::prev(i);
	}
public:
	// Individual allocators use these to talk to the implementation
	T* get_slab_entry() {
		slab* s = get_best_slab();
		if (s->free()) {
			// If free, it won't be now. Move it to partial.
			slabs_partial.splice(slabs_partial.begin(),
				       	slabs_free, s->link);
			slabs_sort.insert(s);
			
		}
		T* ret = s->get();
		if (s->full()) {
			slabs_full.splice(slabs_full.begin(), slabs_partial,
				       	s->link);
			// If we just filled up the hot slab, we need a new
			// one
			if (s == hot_slab)
				hot_slab = nullptr;
		}
		return ret;
	}

	void put_slab_entry(T* p) {
		slab* s = find_slab(p);
		if (s->full())
			slabs_partial.splice(slabs_partial.begin(),
					slabs_full, s->link);
		s->put(p);
		if (s->free()) {
			slabs_free.splice(slabs_free.begin(), slabs_partial,
				       	s->link);
			slabs_sort.erase(s);
		}
	}

	static slab_allocator_base& get() {
		static slab_allocator_base sab;
		return sab;
	}
	// Trims any free slabs if slack memory gets too big. Returns false if
	// we had no memory to free
	static bool trim_slabs() {
		if (get().slabs_free.empty())
			return false;
		for (const auto& a_slab : get().slabs_free)
			delete a_slab;
		get().slabs_free.clear();
		return true;
	}
};

template <typename T>
class slab_allocator {
public:
	typedef T value_type;

	slab_allocator() = default;
	slab_allocator(const slab_allocator& other);
	template <typename U>
	slab_allocator(const slab_allocator<U>& other);

	template <typename U>
	struct rebind { typedef slab_allocator<U> other; };

	~slab_allocator() = default;

	T* allocate(std::size_t n);
	void deallocate(T* p, std::size_t n);
};

template <typename T>
inline T* slab_allocator<T>::allocate(std::size_t n) {
	// For any array allocations, use ::new. Rationale: Shouldn't use slab
	// allocator :)
	if (n > 1) {
		return (T*)::new unsigned char[n * sizeof(T)];
	}
	return slab_allocator_base<T>::get().get_slab_entry();
}

template <typename T>
inline void slab_allocator<T>::deallocate(T* p, std::size_t n) {
	// For array deletes, use ::delete
	if (n > 1) {
		::delete p;
		return;
	}
	slab_allocator_base<T>::get().put_slab_entry(p);
}
