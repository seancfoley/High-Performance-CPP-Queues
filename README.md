# High Performance C++ Queues
High performance C++ implementation of producer/consumer queue architecture

## Fundamentals to this implementation
* the group of consumers and the group of producers do not synchronize between each other.  Contention takes place only when the queue is empty to notify the readers when the queue becomes non-empty.  Otherwise, the readers and writers operate with no contention.
* choose either single or multi-threaded producers
* choose either single or multi-threaded consumers
* memory allocations are minimized or avoided entirely: addition to the queues is done by assignment copying (especially useful for stack-allocated objects and also especially useful to avoid reference counting or other memory tracking).  Removals from the queue are non-copy: they simply use a reference to the entry in the queue itself, and increment a queue counter when done to free up the queue entry.
* The queues are circular but resize as necessary if consumers are not keeping pace with producers, up until a max size limit upon which the producers will block as necessary
* designed to be fast
* easy to customize to enqueue any data types.  

## Source folders

For the queues themselves, all you need is the base and queue source folders, as well as the threading folder which contains basic wrappers for mutexes and conditional variables.  The consumer folder is optional, containing code to maintain a set of queue consumer worker threads, also requiring the thread wrapper class in the threading source folder.  Finally, the sample folder is sample code showing how to pull it all together.

## Sample code
See the [sample](https://github.com/seancfoley/High-Performance-CPP-Queues/tree/master/ProcessingQueues/src/sample) folder for an implementation using two data types that can be enqueued (SampleQueueEntry1 and 2), and some test code which starts up writer threads, a queue processor engine with multiple readers, a single consumer class SampleQueueEntryConsumer used by the queue consumer threads, and adds numerous entries to the queue for consumption in testQueues.  The sample code makes use of the [consumer folder](https://github.com/seancfoley/High-Performance-CPP-Queues/tree/master/ProcessingQueues/src/consumer), which includes a QueueProcessor engine which contains a queue (by default multi-reader and multi-writer) and also manages a set of consumer threads to pull entries off the queue.



### Platforms
* Written in portable code for all platforms
* In a couple of spots it uses g++ atomic built-ins __sync_lock_test_and_set and __sync_fetch_and_add, see [atomic built-ins documentation](https://gcc.gnu.org/onlinedocs/gcc-4.4.5/gcc/Atomic-Builtins.html)
* Developed in Eclipse Mars 2 on Linux using Eclipse CDT and compiling with g++, it can be easily imported into an Eclipse workspace
