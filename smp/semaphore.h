//============================================================================
//                                  libcpp-util
//                   A simple odds-n-ends library for C++11
//
//         Licensed under modified BSD license. See LICENSE for details.
//============================================================================

#ifndef LIBCPP_UTIL_SEMAPHORE_H
#define LIBCPP_UTIL_SEMAPHORE_H

#include "libcpp-util/smp/spinlock.h"

#include <condition_variable>

namespace cpputil {

class semaphore {
private:
	std::condition_variable_any cv;
	unsigned count;
	unsigned waiters;
	spinlock s;

	semaphore(const semaphore&) = delete;
	semaphore& operator=(const semaphore&) = delete;
public:
	semaphore(unsigned initial_count) 
		: count(initial_count), waiters(0) {
	}

	~semaphore() = default;

	void post() {
		std::unique_lock<spinlock> lock(s);
		count++;
		if (waiters > 0) {
			waiters--;
			lock.unlock();
			cv.notify_one();
		}
	}

	void wait() {
		std::unique_lock<spinlock> lock(s);
		if (count > 0) {
			count--;
		} else {
			waiters++;
			while (count == 0)
				cv.wait(lock);
			count--;
		}
	}

	bool try_wait() {
		std::lock_guard<spinlock> lock(s);
		if (count > 0) {
			count--;
			return true;
		} else {
			return false;
		}
	}

	unsigned value() const {
		return count;
	}
};

}
#endif
