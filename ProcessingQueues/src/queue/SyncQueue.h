/*
 * SyncQueue.h
 *
 *  Created on: Sep 23, 2010
 *      Author: sfoley
 */

#ifndef SYNCQUEUE_H_
#define SYNCQUEUE_H_

#include "SyncReaderList.h"
#include "SyncWriterQueue.h"
#include "ThreadInfo.h"

namespace hpqueue {

/*
 * A multi-thread reader and optionally multi-thread writer queue that has no locking between the group of readers and the group of writers.
 *
 * There exists locking between readers, and there exists locking between writers.
 *
 * The queue is non-copying for readers.  When an entry is read by a thread,
 * it is not available for writing until the same thread returns for another entry.
 *
 * Reading and writing move from lower to higher indices in the queue.
 * For 3 threads, typically a queue of 15 slots looks like:
 * index: 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
 *       [E  E  R  R  R  W  W  W  W  W  E  E  E  E  E  E ]
 * E: empty, available for writing
 * W: written slots, ready for reading
 * R: slots being used by reading threads
 *
 * If there are n threads, then a thread t1 which does a read that takes a long time to return may result in
 * more than n slots in use for reading at a time, as it lags behind the other threads t2, t3 to tn
 * which have each handled multiple reads in the same time.
 *
 * The queue's read index is the lowest/leftmost index to be read, the one that has been in the queue the longest.
 *
 * For example, if there are three threads reading, then the queue may look like:
 * index: 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
 *       [W  W* R  E  E  R  R  W  W  W  W  W  W  W  W  W ]
 * Here we can see the queue is fill, since the slot at index 2, which is one above the next slot for writing at index 1,
 * is currently in use by a reading thread t1.  The other reading threads have been a little quicker and are 3 and 4 slots ahead,
 * leaving two empty slots that are not available for writing.  When t1 returns and no longer needs the slot at index 2,
 * it will open up at least 3 slots for writing.
 *
 * As in other queues, the queue is empty when the queue's read index and write index are the same.
 * It is full when the queue's read index is one above the queue's write index.
 *
 * In this queue, each reading thread maintains its own read index in addition to the read index stored in the queue.
 *
 */
class SyncQueue: public SyncWriterQueue {

	bool syncWriter;

	QueueEntryBase &removeSlot(ReaderIndex &readerIndex);

public:
	/**
	 * This class maintains a linked list of indices, each one indicating an entry of the underlying queue currently being read.
	 * Each reader thread has a single ReaderIndex object that will be added to the queue's list when the thread is reading from the queue.
	 * When a thread comes back to the underlying queue after reading an entry,
	 * its ReaderIndex object is updated, giving the thread an updated index in the queue to read next.
	 */
	SyncReaderList readList;

	SyncQueue(
			unsigned int queueSize,
			ProcessingQueueDataArray *dataEntries,
			pthreadWrapper::Mutex *stdoutLock,
			bool syncWriter = true) :
		SyncWriterQueue(queueSize, dataEntries, stdoutLock),
		syncWriter(syncWriter),
		readList(*this, stdoutLock) {}

	virtual ~SyncQueue() {}

	bool isEmpty(ReaderIndex &);

	QueueEntryBase &remove(ReaderIndex &readerIndex);

	int add(QueueEntryBase &entry);

	void resize(unsigned int newSize);

	void startAccess(ReaderIndex &readerIndex) {
		if(debug) {
			ThreadInfo threadInfo;
			threadInfo.initAsCurrentThread();
			stdoutLock->acquire();
			std::cout << threadInfo.getThreadId() << " starting use of queue, read index " << readIndex << " write index " << writeIndex << ", read list is: "<< readList << std::endl;
			stdoutLock->release();
		}

		readList.add(readerIndex);

		if(debug) {
			ThreadInfo threadInfo;
			threadInfo.initAsCurrentThread();
			stdoutLock->acquire();
			std::cout << threadInfo.getThreadId() << " using queue, read index " << readIndex << " write index " << writeIndex << ", read list is: "<< readList << std::endl;
			stdoutLock->release();
		}
	}

	void endAccess(ReaderIndex &readerIndex) {
		if(debug) {
			ThreadInfo threadInfo;
			threadInfo.initAsCurrentThread();
			stdoutLock->acquire();
			std::cout << threadInfo.getThreadId() << " ending use of queue, read index " << readIndex << " and write index " << writeIndex << ", reader index " << readerIndex << " and read list is: "<< readList << std::endl;
			stdoutLock->release();
		}

		readList.remove(readerIndex);

		if(debug) {
			ThreadInfo threadInfo;
			threadInfo.initAsCurrentThread();
			stdoutLock->acquire();
			std::cout << threadInfo.getThreadId() << " no longer using queue, read index " << readIndex << ", read list is: "<< readList << std::endl;
			stdoutLock->release();
		}
	}

	void setDebug(bool debug);
};

}

#endif /* SYNCQUEUE_H_ */
