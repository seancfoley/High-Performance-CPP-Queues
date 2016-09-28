/*
 * ReaderWriterQueue.h
 *
 *  Created on: Sep 23, 2010
 *      Author: sfoley
 */

#ifndef READERWRITERQUEUE_H_
#define READERWRITERQUEUE_H_

#include "ProcessingQueue.h"
#include "QueueConstants.h"
#include "QueueStats.h"
#include "threading/Mutex.h"

namespace hpqueue {

/*
 * A single reader and single writer queue that has no locking between the reader and the writer.
 *
 * The queue is non-copying for readers.  When an entry is read,
 * it is not available for writing until the reader thread adjusts the readIndex of the queue
 * when returning for another entry.
 *
 * Typically a queue of 15 slots looks like:
 * index: 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
 *       [E  E  R  W  W  W  W  W  W  W  E  E  E  E  E  E ]
 * E: empty, available for writing
 * W: written slots, ready for reading
 * R: slot being used by reading thread
 *
 * The queue is circular and wraps around the end to the beginning.
 *
 * The slot indicated by R indicates the leftmost slot to be read,
 * the slot to be read which has been populated for the longest time.
 *
 *
 * The following is an example of a full queue of 15 slots:
 * index: 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
 *       [W  W* R  W  W  W  W  W  W  W  W  W  W  W  W  W ]
 * Here the writer just wrote into the slot at index 0 and the next slot is index 1,
 * one below the slot being used by the reader.  We cannot allow the writer to write to
 * the slot at index 1, even though it is unused, because we need to distinguish a
 * full queue from an empty queue.  When the read index and write index are the same, the
 * queue is empty.  When the write index is 1 below the read index, as in this example,
 * the queue is full.  So there is always one slot unused when the queue is full.
 *
 * When the queue becomes full, the queue is immediately resized by a writing thread
 * as in this example where we have added 10 new slots:
 * index: 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
 *       [W  W* E  E  E  E  E  E  E  E  E  E  R  W  W  W  W  W  W  W  W  W  W  W  W  W  ]
 *
 * All the slots in the range from the next index to read to the latter half of the queue are moved to new indices.
 * In the example, this means the slots at indices 2 to 15 have been copied to the end of the new queue section.
 * The remaining slots in the old queue remain at the same index in the resized queue.
 * The next slot for writing remains at index 1, while the next slot for reading is now at index 12.
 */
class ReaderWriterQueue: public ProcessingQueue, public QueueConstants {
	friend class SyncReaderList; //for access to readIndex

protected:
	/*
	 * tracks queue size, entries added and entries removed.
	 */
	QueueStats stats;

	/*
	 * indicates the index above the current lowest index being read (an index which is therefore non-writable)
	 */
	volatile int readIndex;

	/*
	 * indicates the next index to write
	 */
	volatile int writeIndex;

	pthreadWrapper::Mutex *stdoutLock;

	/*
	 * Performs the actual data copying for an add operation.
	 */
	void insert(QueueEntryBase &, QueueEntryBase *, QueueEntryBase *&);

	/*
	 * Determines where the data should be stored and performs the actual data copying for an add operation.
	 */
	bool insert(QueueEntryBase &);

	/*
	 * Inserts if the queue is not full.
	 */
	int tryInsert(QueueEntryBase &);

	/*
	 * Returns -1 if the queue is full or the next write index otherwise.
	 */
	virtual int isFull();

	inline int adjustIndexForComparison(int index, int readIndex) {
		/* readIndex is considered the  base of the queue when doing comparisons of emptiness or fullness */
		return (index < readIndex) ? (index + currentSize) : index;
	}

	inline int adjustIndexForComparison(int index) {
		return adjustIndexForComparison(index, readIndex);
	}

	bool debug;

public:
	ReaderWriterQueue(
			unsigned int queueSize,
			ProcessingQueueDataArray *dataEntries,
			pthreadWrapper::Mutex *stdoutLock,
			bool deleteQueueData = false):
		ProcessingQueue(queueSize, dataEntries, deleteQueueData),
		stats(queueSize),
		readIndex(0),
		writeIndex(0),
		stdoutLock(stdoutLock),
		debug(false) {}

	virtual ~ReaderWriterQueue() {}

	/*
	 * Attempts to add the data in entry to the queue.
	 * If there is space, the data will be copied to the location indicated by the pointer,
	 * and the queue index for the data will be returned.
	 * Otherwise, IS_FULL is returned if the queue is full,
	 * or CANNOT_ADD is returned if the queue does not know how to store the given entry.
	 */
	virtual int add(QueueEntryBase &);

	/*
	 * Returns whether the queue is empty at a specific next read index,
	 * which is determined by the queue itself, or by the provided ReaderIndex object.
	 * For a sequential queue in which the next read index is determined by the
	 * queue itself, the ReaderIndex object is updated to indicate the read index.
	 */
	virtual bool isEmpty(ReaderIndex &);

	virtual QueueEntryBase &remove(ReaderIndex &);

	virtual void resize(unsigned int newSize);

	/**
	 * This method is not synchronized (intentionally) and thus does not give an exact answer when the queue is being modified.
	 */
	int getNumElements() {
		return writeIndex - readIndex;
	}

	virtual void print(const std::string &prefix = "");

	void setDebug(bool debug);

	QueueStats &getStats();

	static void printStats(FILE *fp, const std::string &queueName, const QueueStats &stats);
};

}

#endif /* READERWRITERQUEUE_H_ */
