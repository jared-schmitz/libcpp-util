#ifndef LIBCPP_CONTIGUOUS_SET_H
#define LIBCPP_CONTIGUOUS_SET_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <vector>

template <class Key, class Compare = std::less<Key>>
class contiguous_set {
private:
	std::vector<Key> storage;
	bool equivalent(const Key& k1, const Key& k2) {
		return !key_comp()(k1, k2) && !key_comp()(k2, k1);
	}
public:
	typedef Key key_type;
	typedef Key value_type;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef Compare key_compare;
	typedef Compare value_compare;
	typedef typename decltype(storage)::allocator_type allocator_type;
	typedef value_type& reference_type;
	typedef const value_type& const_reference_type;
	typedef typename std::allocator_traits<allocator_type>::pointer pointer;
	typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;
	typedef typename decltype(storage)::iterator iterator;
	typedef typename decltype(storage)::const_iterator const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	// Constructors, assignment, destructor
	// TODO: All the ctors
	contiguous_set& operator=(const contiguous_set&) = default;
	contiguous_set& operator=(contiguous_set&&) = default;
	contiguous_set& operator=(std::initializer_list<value_type> ilist) {
		storage.assign(ilist);
	}

	~contiguous_set() = default;

	// Iterators
	iterator begin() { return storage.begin(); }
	const_iterator begin() const { return storage.begin(); }
	const_iterator cbegin() const { return storage.cbegin(); }

	iterator end() { return storage.end(); }
	const_iterator end() const { return storage.end(); }
	const_iterator cend() const { return storage.cend(); }

	reverse_iterator rbegin() { return storage.rbegin(); }
	const_reverse_iterator rbegin() const { return storage.rbegin(); }
	const_reverse_iterator crbegin() const { return storage.crbegin(); }

	reverse_iterator rend() { return storage.rend(); }
	const_reverse_iterator rend() const { return storage.rend(); }
	const_reverse_iterator crend() const { return storage.crend(); }

	// Accessors for functions
	bool empty() const { return storage.empty(); }
	size_type size() const { return storage.size(); }
	size_type max_size() const { return storage.max_size(); }

	key_compare key_comp() const { return Compare(); }
	value_compare value_comp() const { return Compare(); }

	// Modifiers
	void clear() { storage.clear(); }
	size_type erase(const key_type& key) {
		auto I = find(key);
		if (I != end())
			storage.erase(I);
	}

	void swap(contiguous_set& other) {
		storage.swap(other.storage);
	}

	// insert, emplace, emplace_hint
	std::pair<iterator, bool> insert(const value_type& value);
	std::pair<iterator, bool> insert(value_type&& value);

	iterator insert(const_iterator hint, const value_type& value);
	iterator insert(const_iterator hint, value_type&& value);

	template <class InputIt>
	void insert(InputIt first, InputIt last) {
		// Procedure is to insert the new elements at the end, sort
		// them, then do an inplace merge of the entire vector.
		size_type old_size = size();
		storage.insert(end(), first, last);
		std::sort(begin() + old_size, end(), Compare());
		std::inplace_merge(begin(), begin() + old_size, end(),
				Compare());
	}

	void insert(std::initializer_list<value_type> ilist) {
		insert(ilist.begin(), ilist.end());
	}

	template <class... Args>
	std::pair<iterator, bool> emplace(Args&&... args) {
		// Handle the annoying case where the type may not be moveable.
		value_type tmp(std::forward<Args>(args)...);
		auto I = lower_bound(tmp);
		if (I == end()) {
			storage.emplace(I, std::forward<Args>(args)...);
			return std::make_pair(I, true);
		} else if (equivalent(*I, tmp)) {
			return std::make_pair(I, false);
		} else {
			storage.emplace(I, std::forward<Args>(args)...);
			return std::make_pair(I, true);
		}
	}

	template <class... Args>
	iterator emplace_hint(const_iterator, Args&&... args) {
		// XXX: We can't use the hint efficiently because of the
		// contiguous nature of the storage. :)
		return emplace(std::forward<Args>(args)...).first;
	}

	// Search operations we need to implement
	size_type count(const key_type& key) const {
		auto result = equal_range(key);
		return std::distance(result.first, result.second);
	}

	iterator find(const Key& key) {
		auto I = lower_bound(key);
		if (I == end() || key_comp()(key, *I))
			return I;
		return end();
	}
	const_iterator find(const Key& key) const {
		return const_cast<contiguous_set<Key, Compare>*>(this)->find(key);
	}

	iterator upper_bound(const Key& key) {
		return std::upper_bound(begin(), end(), key, Compare());
	}
	const_iterator upper_bound(const Key& key) const {
		return std::upper_bound(begin(), end(), key, Compare());
	}

	iterator lower_bound(const Key& key) {
		return std::lower_bound(begin(), end(), key, Compare());
	}
	const_iterator lower_bound(const Key& key) const {
		return std::lower_bound(begin(), end(), key, Compare());
	}

	std::pair<iterator, iterator> equal_range(const Key& key) {
		return std::equal_range(begin(), end(), key, Compare());
	}

	std::pair<const_iterator, const_iterator>
	equal_range(const Key& key) const {
		return std::equal_range(begin(), end(), key, Compare());
	}

};

#endif
