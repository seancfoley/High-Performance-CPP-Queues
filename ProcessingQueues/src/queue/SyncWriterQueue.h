/*
 * SyncWriterQueue.h
 *
 *  Created on: Sep 23, 2010
 *      Author: sfoley
 */

#ifndef SYNCWRITERQUEUE_H_
#define SYNCWRITERQUEUE_H_

#include "ReaderWriterQueue.h"
#include "threading/Mutex.h"

namespace hpqueue {

/*
 * A single reader and multiple writer queue that has no locking between the reader and the writers.
 *
 * There exists locking between writers.
 */
class SyncWriterQueue: public ReaderWriterQueue {
protected:
	pthreadWrapper::Mutex addMutex;
public:
	SyncWriterQueue(
			unsigned int queueSize,
			ProcessingQueueDataArray *dataEntries,
			pthreadWrapper::Mutex *stdoutLock,
			bool deleteQueueData = false) :
		ReaderWriterQueue(queueSize, dataEntries, stdoutLock, deleteQueueData) {}

	virtual ~SyncWriterQueue() {}
	virtual int add(QueueEntryBase &entry);
};

}

#endif /* SYNCWRITERQUEUE_H_ */
