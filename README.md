# High Performance C++ Queues
High performance C++ implementation of producer/consumer queue architecture

## Fundamentals to this implementation
* the group of consumers and the group of producers do not synchronize between each other
* choose either single or multi-threaded producers
* choose either single or multi-threaded consumers
* memory allocations are minimized or avoided entirely: addition to the queues is done by assignment copying (especially useful for stack allocated objects and especially useful to avoid reference counting or other memory tracking).  Removals from the queue simply use a reference to the entry in the queue itself. 
* the circular queue resizes as necessary if consumers are not keeping pace with producers at all times
* fast
* easy to customize to enqueue specific data types


