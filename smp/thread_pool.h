#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <cassert>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

// TODO Make this a base class and move the queueing into a subclass. Have one
// version for many short tasks (circular FIFO) and one for long tasks
class thread_pool {
private:
	std::vector<std::thread> threads;
	std::deque<std::function<void()>> tasks;

	std::mutex task_lock;
	std::condition_variable cv;
	bool open;

	void thread_loop();
public:
	thread_pool(size_t maxThreads);
	~thread_pool();

	void close();
	void close_and_drain();

	template <class F, class... Args>
	std::future<typename std::result_of<F(Args...)>::type>
	add_task(F&& f, Args&&... args);
};

inline thread_pool::thread_pool(size_t maxThreads) : open(true) {
	// TODO: Maybe lazily spawn threads?
	threads.reserve(maxThreads);
	for (size_t i = 0; i < maxThreads; i++)
		threads.emplace_back(std::thread(&thread_pool::thread_loop,
					this));
}

inline thread_pool::~thread_pool() {
	close_and_drain();
}

inline void thread_pool::close() {
	// XXX: We don't use an atomic boolean because all other reads are
	// guarded by the mutex and it's only closed once.
	task_lock.lock();
	open = false;
	task_lock.unlock();
}

inline void thread_pool::close_and_drain() {
	// Close and...
	close();

	// ...wake all threads and wait until they exit 
	cv.notify_all(); 
	for (auto& t : threads)
		t.join();
}

inline void thread_pool::thread_loop() {
	while (1) {
		std::unique_lock<std::mutex> l(task_lock);
		while (open && tasks.empty())
			cv.wait(l);
		if (!open && tasks.empty())
			break;
		auto task = tasks.back();
		tasks.pop_back();
		l.unlock();
		task();
	}
}

template<class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type>
thread_pool::add_task(F&& f, Args&&... args) 
{
	using return_type = typename std::result_of<F(Args...)>::type;

	assert(open && "Adding task to terminating thread pool");

	auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(f),
				std::forward<Args>(args)...));

	std::future<return_type> res = task->get_future();
	assert(res.valid());

	// Shove onto queue
	std::unique_lock<std::mutex> l(task_lock);
	tasks.emplace_back([task]() { (*task)(); });
	l.unlock();
	cv.notify_one();
	return res;
}

#endif
