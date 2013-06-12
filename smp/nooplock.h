//============================================================================
//                                  libcpp-smp
//                   A simple threading supplement for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_SMP_NOOPLOCK_H
#define LIBCPP_SMP_NOOPLOCK_H

#include <chrono>

namespace libcpp-smp {

// Lock that can be dropped into a class that has a synchronization aspect but
// there is no locking needed
class noop_lock {
	// There's no reason not to allow this to be moved or copied, but it
	// would allow programming error that might not be caught until a
	// conventional lock is used, so just disallow it.
	noop_lock(const noop_lock&) = delete;
	noop_lock& operator=(noop_lock&&) = delete;

public:
	noop_lock() = default;
	~noop_lock() = default;

	void lock() {}

	bool try_lock() { return true; }

	template <class Rep, class Period>
	bool try_lock_for(const std::chrono::duration<Rep,Period>&) {
		return true;
	}


	template <class Clock, class Duration>
	bool try_lock_until(const std::chrono::time_point<Clock, Duration>&) {
		return true;
	}

	void unlock() {}
};

}
#endif
