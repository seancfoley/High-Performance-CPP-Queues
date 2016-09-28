/*
 * ProcessingQueue.h
 *
 *  Created on: Sep 7, 2010
 *      Author: sfoley
 */

#ifndef PROCESSINGQUEUE_H_
#define PROCESSINGQUEUE_H_

#include "ProcessingQueueDataArray.h"
#include "ReaderIndex.h"

/*
 * Returns the max number of entries allowed for the queue, given the size of each entry.
 * entrySize is the number of bytes per entry in the queue.
 */
#define MAX_QUEUE_SIZE_BYTES(maxBytes, entrySize) ((maxBytes) / (entrySize))
#define MAX_QUEUE_SIZE(entrySize) MAX_QUEUE_SIZE_BYTES(500 * 1024 * 1024, entrySize)


namespace hpqueue {

/*
 * The base class for a high performance queue.
 *
 * Reading and writing to the queue moves from lower to higher indices, and the queue is circular.
 * The queue can grow in size.  The queue maintains a list of pointers pointing to the entries,
 * which are of type QueueEntryBase.  However, the entries themselves are stored in an object of
 * type ProcessingQueueDataArray.  When creating a queue, the caller must provide a subclass of
 * ProcessingQueueDataArray so that the queue can be customized to store only certain types of QueueEntry.
 *
*/
/*
	ProcessingQueue, which maintains the fields requires to store requests,
	is the base class of the queue type hierarchy:

	ProcessingQueue
		<-- ReaderWriterQueue
			<-- SyncWriterQueue
				<-- SyncQueue
 */
class ProcessingQueue {
	bool deleteQueueData;
protected:
	QueueEntryBase **queueData;
	ProcessingQueueDataArray *dataEntries;
	volatile unsigned int currentSize;

public:
	ProcessingQueue(unsigned int queueSize, ProcessingQueueDataArray *dataEntries, bool deleteQueueData = false) :
		deleteQueueData(deleteQueueData),
		queueData(new QueueEntryBase *[queueSize]),
		dataEntries(dataEntries),
		currentSize(queueSize) {
		dataEntries->resize(queueSize);
	}

	virtual ~ProcessingQueue() {
		if(deleteQueueData) {
			delete dataEntries;
		}
		delete[] queueData;
	}

	virtual int add(QueueEntryBase &entry) = 0;

	/*
	 * Remove an entry from the queue.
	 * The entry that is removed depends on the queue type: either the first in (FIFO), or a selected entry.
	 * If it is FIFO, then the readerIndex argument is updated with the queue index removed.
	 * Otherwise, the queue index in the readerIndex indicates which entry to remove.
	 */
	virtual QueueEntryBase &remove(ReaderIndex &readerIndex) = 0;

	static inline int nextIndex(int index, unsigned int currentSize) {
		return (index + 1) % currentSize;
	}

	inline int nextIndex(int index) {
		return nextIndex(index, currentSize);
	}

	unsigned int getCurrentSize() {
		return currentSize;
	}

	virtual void resize(unsigned int size) = 0;

	virtual unsigned int getEntrySize() {
		return dataEntries->getEntrySize() + sizeof(QueueEntryBase *);
	}

	unsigned int getNewQueueSize();
};

}

#endif /* PROCESSINGQUEUE_H_ */
