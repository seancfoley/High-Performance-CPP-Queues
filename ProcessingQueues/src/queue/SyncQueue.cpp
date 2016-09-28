/*
 * SyncReaderQueue.cpp
 *
 *  Created on: Sep 23, 2010
 *      Author: sfoley
 */

#include "SyncQueue.h"

using namespace std;

namespace hpqueue {


int SyncQueue::add(QueueEntryBase &entry) {
	if(syncWriter) {
		return SyncWriterQueue::add(entry);
	}
	return ReaderWriterQueue::add(entry);
}

bool SyncQueue::isEmpty(ReaderIndex &readerIndex) {
	/*
	 * When the queue readIndex is one above the writeIndex, the queue is full.
	 * When the queue readIndex matches writeIndex, the queue is empty.
	 * When the readIndex of the current thread matches or is below the the writeIndex,
	 * but matches or is above the queue readIndex, then the thread's cell is empty.
	 */
	return (readerIndex.isEmpty = (adjustIndexForComparison(writeIndex) <= adjustIndexForComparison(readerIndex.index)));
}

QueueEntryBase &SyncQueue::removeSlot(ReaderIndex &readerIndex) {
	bool done = readerIndex.isDone = !isEmpty(readerIndex);
	if(!done) {
		/* the slot is empty */
		return QueueEntryBase::nullEntry;
	}

	QueueEntryBase &result = *queueData[readerIndex.index];
	return result;
}

QueueEntryBase &SyncQueue::remove(ReaderIndex &readerIndex) {
	/*
	 * If our currently assigned slot has been populated,
	 * readerIndex.isDone tells us whether it was previously handled.
	 */
	if(!readerIndex.isDone) {
		/* previously our slot had nothing in it, so we check that same slot again */
		QueueEntryBase &result = removeSlot(readerIndex);
		return result;
	}

	/* Need to move ahead to a new slot: when last checked our slot had something in it, so we've already used it */
	if(readList.moveToFront(readerIndex)) {
		QueueEntryBase &result = removeSlot(readerIndex);
		return result;
	}

	/* just stay where we are, the back of the read list needs to move */
	return QueueEntryBase::nullEntry;
}

void SyncQueue::resize(unsigned int size) {
	int oldSize = currentSize;
	int oldReadIndex = readIndex;
	SyncWriterQueue::resize(size);

	int adjustment = size - oldSize;
	for(int i=oldReadIndex; adjustIndexForComparison(i, oldReadIndex) < adjustIndexForComparison(writeIndex, oldReadIndex); i = nextIndex(i, oldSize)) {
		int newIndex = i;
		if(i > writeIndex) {
			newIndex += adjustment;
		}
	}
	readList.adjust(adjustment, oldReadIndex);
}

void SyncQueue::setDebug(bool debug) {
	SyncWriterQueue::setDebug(debug);
	readList.setDebug(debug);
}

}

