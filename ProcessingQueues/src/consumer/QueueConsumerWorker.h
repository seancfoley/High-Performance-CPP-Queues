/*
 * ConsumingWorker.h
 *
 *  Created on: Sep 22, 2010
 *      Author: sfoley
 */

#ifndef CONSUMINGWORKER_H_
#define CONSUMINGWORKER_H_

#include "Worker.h"
#include "Consumer.h"
#include "queue/SyncQueue.h"
#include "queue/ResizeControls.h"

namespace hpqueue {

class QueueConsumerWorker: public Worker {

	pthreadWrapper::Mutex *stdoutLock;

	/**
	 * For a queue processor with a single reader consuming entries, you can use this as the queueAccess field with a SyncWriterQueue.
	 *
	 * For a queue processor with a single reader consuming entries and a single writer producing entries,
	 * you can use as the queueAccess field this with a ReaderWriterQueue.
	 */
	struct QueueAccess {
		ReaderWriterQueue *queue;
		ReaderIndex readerIndex;
		unsigned int removedCount;

		QueueAccess(ReaderWriterQueue *queue, int identifier) : queue(queue), readerIndex(identifier), removedCount(0) {}

		bool isEmpty() {
			return queue->isEmpty(readerIndex);
		}

		QueueEntryBase &remove() {
			return queue->remove(readerIndex);
		}

		virtual void updateStats() {
			/* we do an atomic swap so that calls to incrementRemovedCount require no synchronization */
			unsigned int *ptr = &removedCount;
			int removed = __sync_lock_test_and_set(ptr, 0);
			__sync_lock_release(ptr);
			queue->getStats().incrementRemovedCount(removed);
		}

		virtual void incrementRemovedCount() {
			removedCount++;
		}
	};

	/**
	 * Multiple workers access a SyncQueue
	 */
	struct SharedQueue : public QueueAccess {
		SyncQueue *syncQueue;

		SharedQueue(SyncQueue *queue, int identifier) : QueueAccess(queue, identifier), syncQueue(queue) {}

		void startAccess() {
			syncQueue->startAccess(readerIndex);
		}

		void endAccess() {
			syncQueue->endAccess(readerIndex);
		}
	} queueAccess;

	Consumer *consumer;

	/* the number of workers, including this one */
	int numWorkers;

	bool doWork();

	bool isWork();

	void init();

	void finalize();

public:

	QueueConsumerWorker(
			int identifier,
			SyncQueue *sharedQueue,
			int numWorkers,
			Consumer *consumer,
			pthreadWrapper::Mutex *stdoutLock) :
		Worker(identifier, NULL),
		stdoutLock(stdoutLock),
		queueAccess(sharedQueue, identifier),
		consumer(consumer),
		numWorkers(numWorkers) {}

	virtual ~QueueConsumerWorker() {}

	void setDebug(bool debug);

	void updateStats();
};

}

#endif /* CONSUMINGWORKER_H_ */
