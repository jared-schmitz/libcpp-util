#ifndef LIBCPP_SMP_SEMAPHORE_H
#define LIBCPP_SMP_SEMAPHORE_H

#include "libcpp-smp/smp/spinlock.h"

#include <condition_variable>

namespace libcpp-smp {

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
};

}
#endif
