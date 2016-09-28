/*
 * SyncReaderList.h
 *
 *  Created on: Sep 30, 2010
 *      Author: sfoley
 */

#ifndef SYNCREADERLIST_H_
#define SYNCREADERLIST_H_

#include "ReaderIndex.h"
#include "ReaderWriterQueue.h"
#include "threading/Mutex.h"

namespace hpqueue {

/**
 * This class handles multiple-thread read access from a SyncReaderQueue.
 * Each reader from the queue has its own ReaderIndex object.
 * All the ReaderIndex objects for the queue form a linked list.  When a reader attempts to read from the queue, its
 * ReaderIndex object is assigned an index in the queue to read from, which is the next index at the front of the queue
 * (as indicated by front) that has not yet been read.  The back of the queue is tracked as well,
 * which tells us the last entry that has not yet been read from the queue,
 * which is tracked by the queue's readIndex field, indicating when the queue is full.
 *
 * Because of the linked list, threads which wish to become readers must call add, and threads which no longer wish to be
 * readers must call remove.  When a thread has finished reading from a queue entry, it can be assigned a new queue entry
 * by calling moveToFront.
 */
class SyncReaderList {
	friend class SyncQueue;

	pthreadWrapper::Mutex indexMutex;
	ReaderWriterQueue &queue;
	ReaderIndex *front;
	ReaderIndex *back;
	pthreadWrapper::Mutex *stdoutLock;

	void moveOut(ReaderIndex &index);

	bool debug;

public:
	SyncReaderList(ReaderWriterQueue &queue, pthreadWrapper::Mutex *stdoutLock) :  indexMutex(), queue(queue), front(NULL), back(NULL), stdoutLock(stdoutLock), debug(false) {}
	virtual ~SyncReaderList() {}
	bool moveToFront(ReaderIndex &index);
	void remove(ReaderIndex &index);
	void add(ReaderIndex &index);
	friend std::ostream& operator <<(std::ostream &outputStream, SyncReaderList &readerList);
	void adjust(int adjustment, int queueReadIndex);
	void setDebug(bool debug);
};

}

#endif /* SYNCREADERLIST_H_ */
