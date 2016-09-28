/*
 * ProcessingQueueDataArray.h
 *
 *  Created on: Dec 22, 2011
 *      Author: sfoley
 */

#ifndef PROCESSINGQUEUEDATAARRAY_H_
#define PROCESSINGQUEUEDATAARRAY_H_

#include "queue/QueueEntryBase.h"

namespace hpqueue {

/**
 * Represents a storage cell at a given index in the queue.
 *
 * Subclasses may customize how they store the data, and subclasses may choose which types can be stored.
 */
struct QueueData {
	QueueData() {}

	virtual ~QueueData() {}

	/**
	 * Returns a pointer to an entry of the same type as the given type in this cell,
	 * or NULL if this cell cannot store entries of the given type.
	 */
	virtual QueueEntryBase *getQueueEntry(QueueEntryBase &entry) = 0;
};

class ProcessingQueueDataArray {
	unsigned int currentSize;

protected:
	QueueData *queueDataEntries;

	QueueData *resize(unsigned int queueSize, QueueData *newDataEntries) {
		QueueData *oldEntries = queueDataEntries;
		queueDataEntries = newDataEntries;
		currentSize = queueSize;
		return oldEntries;
	}

public:
	ProcessingQueueDataArray() :
		currentSize(0),
		queueDataEntries(NULL) {}

	virtual ~ProcessingQueueDataArray() {}

	/*
	 * Returns the queue's entry at the specified index that corresponds to the given entry.
	 *
	 * It will return a pointer to an entry of the same type in the queue at the given index,
	 * or NULL if this queue cannot store entries of the given type.
	 *
	 * Callers can then either read the associated queue entry, or store the given entry
	 * into that slot.
	 */
	QueueEntryBase *getQueueEntry(unsigned int index, QueueEntryBase &entry) {
		if(index < currentSize) {
			return getEntry(index).getQueueEntry(entry);
		}
		return NULL;
	}

	/**
	 * Returns true if the given entry can be stored within this queue.
	 */
	virtual bool isCompatibleEntry(QueueEntryBase &entry) = 0;

	/**
	 * Subclasses must implement this method, returning a reference to an instance of a subclass
	 * of QueueData at the given index in this queue.
	 */
	virtual QueueData &getEntry(unsigned int index) = 0;

	/*
	 * Resizes the data entries.
	 *
	 * The old data entries are returned as a pointer, so that callers may continue to read them.
	 * Callers are responsible for calling delete[] on the returned pointer when the old data
	 * entries are no longer required.
	 */
	virtual QueueData *resize(unsigned int queueSize) = 0;

	/**
	 * delete an array of data entries previously returned by a call to resize.
	 */
	virtual void deleteEntries(QueueData *entries) = 0;

	/**
	 * Returns the size of a given QueueData object for this queue.  This member is used to determine
	 * how much memory is consumed by this queue.
	 */
	virtual unsigned int getEntrySize() = 0;
};

}

#endif /* PROCESSINGQUEUEDATAARRAY_H_ */
