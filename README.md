libcpp-smp
==========

C++11 library containing efficient supplements to the standard thread support library

All the structures attempt to conform to C++ concepts and naming conventions.
The data structures are move-aware and try to place minimal requirements on
contained type.

Synchronization primitives:
- Semaphore
- Spinlock
- Nooplock (implements BasicLockable while providing no synchronization)

Data structures:
- Raw array - Thin wrapper around aligned_storage.
- Circular FIFO - Unsynchronized.
- Blocking single-producer/single-consumer (SPSC) FIFO
- Blocking multi-producer/multi-consumer FIFO
- Thread pool - Provides a work queue model

TODO
- Split TODO to a separate file :)

- Create an abstraction for queue "end policy" to simply mix and match
blocking/nonblocking and single/multiple client use.

- Be allocator aware.

- Add lock-free versions of queues
