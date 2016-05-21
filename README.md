# kissFramework

This is currently a barebones framework aimed primarily towards the developement of lightweight task-parallel (graphical) applications. It features a simple but powerful Job Scheduler, useful synchronisation primitives, and a few other noteable classes such as:

**Containers/Array.h:** Lightweight alternative to stl vector with much faster iterator debugging and designed towards marginally better performance with POD types than stl vector. Written in C++11 but can be trivially ported to C++03.

**Containers/CyclicConcurrentQueue.h:** Lockless Queue which aims for fast performance especially under high contention in a multithreaded environment. Exploits move semantics to avoid runtime allocations, avoids false sharing, and exceptionally suited as a Producer/Consumer queue for task schedulers.

**Memory/DataPointer.h:** An experimental/WIP pointer interface that abstracts away the underlying direct memory access in favor of contiguous data and simpler memory tracking; but at the cost of non-standard new/delete syntax.

**Memory/ThreadStackAllocator.h:** Uses a pre-allocated block of heap memory as thread local storage for those potentially large temporary allocations that can't ideally fit on the stack.

**Reflection/TypeUID:** Template-based runtime 32-bit typeid generator. Also features specialisable Typename() generator and a standalone incremental UID generator that supports asynchronous usage.

**Reflection/EnumReflector:** Derived Reflector.h interface class. Used in conjunction with EnumString.h to automatically generate string values for enums via macro & template recursion. Currently only supports sequential enums and requires un-conventional enum declaration syntax.

There's always more to come, remember to checkout the kissSamples repository for some of the ways this framework can be utilised.
