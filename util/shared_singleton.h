// TODO: Library/license preamble

#ifndef LIBCPP_UTIL_SHARED_SINGLETON
#define LIBCPP_UTIL_SHARED_SINGLETON

#include <atomic>
#include <memory>
#include <mutex>
#include <type_traits>

#include "libcpp-util/smp/spinlock.h"

namespace cpputil {

class shared_singleton_base {
private:
	spinlock lock_;
	std::atomic_bool dead_;

protected:
	shared_singleton_base() : dead_(true) {
	}
	std::unique_lock<spinlock> lock() {
		return std::unique_lock<spinlock>(lock_);
	}

	bool is_dead() const {
		return dead_.load();
	}
	void set_alive() {
		dead_.store(false);
	}
};

template <typename T>
class shared_singleton : public shared_singleton_base {
public:
	using pointer_type = std::shared_ptr<T>;

private:
	using storage_type = typename std::aligned_storage<
	    sizeof(pointer_type), alignof(pointer_type)>::type;
	static storage_type data_;

	shared_singleton(const shared_singleton &) = delete;
	shared_singleton &operator=(const shared_singleton &) = delete;

public:
	shared_singleton() = default;
	~shared_singleton() {
		if (!is_dead())
			reinterpret_cast<pointer_type *>(&data_)
			    ->~pointer_type();
	}

	pointer_type get() {
		if (is_dead()) {
			auto g = lock();
			if (is_dead()) {
				T* temp = new T;
				::new (&data_) pointer_type(temp);
				set_alive();
			}
		}
		return *reinterpret_cast<pointer_type *>(&data_);
	}
};

template <typename T>
typename shared_singleton<T>::storage_type shared_singleton<T>::data_;
}

#endif
