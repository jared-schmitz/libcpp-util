libcpp-util
==========

C++11 library containing classes that I've pieced together in my time doing
lower level systems work.

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

C++14 things:
NB: I am not involved with the C++ working group and I'm sure that my
implementations will be non-conforming in subtle ways. I did it both out of
impatience and curiousity. That said I have made a decent effort to test.
Also I haven't provided workarounds for compilers that haven't caught up with
C++11 fully. If you want to see a reference implementation, they are around on
github.

- dynarray - An array whose size is a runtime constant (it is fixed at
  construction time).
- array_ref - Convenience wrapper to represent contiguous containers.
- string_ref - As above, but for string-like types.
