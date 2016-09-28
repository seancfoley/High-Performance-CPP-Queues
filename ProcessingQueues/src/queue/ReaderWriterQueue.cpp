/*
 * ReaderWriterQueue.cpp
 *
 *  Created on: Sep 23, 2010
 *      Author: sfoley
 */

#include "ReaderWriterQueue.h"

using namespace std;

namespace hpqueue {

int ReaderWriterQueue::isFull() {
	/* The slot just before the current read index is in use by the reader until it reads another */
	int next = nextIndex(writeIndex);
	if(next == readIndex) {
		/* queue is full, cannot write */
		return IS_FULL;
	}
	return next;
}


void ReaderWriterQueue::insert(QueueEntryBase &entry, QueueEntryBase *destination, QueueEntryBase *&destinationPtr) {
	destinationPtr = destination;
	entry.copyTo(destination);
}

bool ReaderWriterQueue::insert(QueueEntryBase &entry) {
	if(dataEntries->isCompatibleEntry(entry)) {
		QueueEntryBase *queueEntry = dataEntries->getQueueEntry(writeIndex, entry);
		if(queueEntry) {
			insert(entry, queueEntry, queueData[writeIndex]);
			return true;
		}
	}
	return false;
}

int ReaderWriterQueue::tryInsert(QueueEntryBase &entry) {
	int nextIndex = isFull();
	if(nextIndex == IS_FULL) {
		return IS_FULL;
	}
	if(!insert(entry)) {
		return CANNOT_ADD;
	}
	return nextIndex;
}

int ReaderWriterQueue::add(QueueEntryBase &entry) {
	int nextIndex = tryInsert(entry);
	if(nextIndex < 0) {
		return nextIndex;
	}
	int currentWriteIndex = writeIndex;
	writeIndex = nextIndex;
	stats.incrementAddedCount();
	return currentWriteIndex;
}

bool ReaderWriterQueue::isEmpty(ReaderIndex &readerIndex) {
	return (readerIndex.isEmpty = (readIndex == writeIndex));
}

QueueEntryBase &ReaderWriterQueue::remove(ReaderIndex &readerIndex) {
	if(isEmpty(readerIndex)) {
		/* queue is empty */
		return QueueEntryBase::nullEntry;
	}
	readerIndex.index = readIndex;
	QueueEntryBase &result = *queueData[readIndex];
	readIndex = nextIndex(readIndex);
	return result;
}

/*
 * We move the front of the queue to the new section:
 * We have:
 * Y  Y  Y  X  Y* Y  Y
 * Y are being read, Y* is the next to read, X is the next entry to be written.
 * which then becomes
 * Y  Y  Y  X  E  E  E  E  E  Y* Y  Y
 * We only move the readIndex and leave the writeIndex as is.
 *
 * If X is at the end, then we simply add the new section:
 * Y* Y  Y  Y  Y  Y  X
 * becomes
 * Y* Y  Y  Y  Y  Y  X  E  E  E  E  E
 */
void ReaderWriterQueue::resize(unsigned int size) {
	if(currentSize >= size) {
		return;
	}
	QueueEntryBase **newQueueData = new QueueEntryBase *[size];
	QueueEntryBase **oldQueueData = queueData;
	QueueData *oldEntries = dataEntries->resize(size);
	int oldSize = currentSize;
	queueData = newQueueData;
	currentSize = size;

	int adjustment = size - oldSize;

	for(int i=readIndex; adjustIndexForComparison(i) < adjustIndexForComparison(writeIndex); i = nextIndex(i, oldSize)) {
		int newIndex = i;
		if(i > writeIndex) {
			newIndex += adjustment;
		}

		QueueEntryBase *oldEntry = oldQueueData[i];
		QueueEntryBase *newEntry = dataEntries->getQueueEntry(newIndex, *oldEntry);
		if(newEntry) {
			insert(*oldEntry, newEntry, newQueueData[newIndex]);
		} else {
			newQueueData[newIndex] = &QueueEntryBase::nullEntry;
		}
	}
	if(readIndex >= writeIndex + 1) {
		readIndex += adjustment;
	}
	delete[] oldQueueData;
	dataEntries->deleteEntries(oldEntries);
	stats.setSize(size);
}

void ReaderWriterQueue::print(const std::string &prefix) {
	int count = 0;
	stdoutLock->acquire();
	std::cout << prefix << " size: " << currentSize << " readIndex: " << readIndex << " writeIndex: " << writeIndex << std::endl;
	for(int i=readIndex; adjustIndexForComparison(i) < adjustIndexForComparison(writeIndex); i = nextIndex(i)) {
		QueueEntryBase *entry = queueData[i];
		std::cout << ++count << " index " << i << " [" << *entry << "]"  << std::endl;
	}
	std::cout << std::endl;
	stdoutLock->release();
}

void ReaderWriterQueue::setDebug(bool debug) {
	this->debug = debug;
}

QueueStats &ReaderWriterQueue::getStats() {
	return stats;
}

}

