#include "thread_pool.h"

#include <cstdio>

void hello_world() {
	puts("Hello world");
}

int goodbye_world() {
	puts("Goodbye world");
	return 3;
}

int main() {
	thread_pool pool(5);

	auto f = pool.add_task(hello_world);

	f.wait();
	return 0;
}
