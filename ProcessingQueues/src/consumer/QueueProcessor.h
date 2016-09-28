/*
 * QueueProcessor.h
 *
 *  Created on: Sep 7, 2010
 *      Author: sfoley
 */

#ifndef QUEUEPROCESSOR_H_
#define QUEUEPROCESSOR_H_

#include <iostream>
#include <vector>
#include <list>
#include <stdexcept>

#include "Worker.h"
#include "Consumer.h"
#include "queue/SyncQueue.h"
#include "sample/SampleDataArray.h"

namespace hpqueue {

/**
 * The queue processor maintains a collection of threads that consumer entries in the queue.
 */
class QueueProcessor {
	unsigned int numWorkers;
	bool isRunningFlag;
	bool isPausedFlag;

	pthreadWrapper::Mutex stdoutLock;//lock to synchronize writing to stdout or stderr
	pthreadWrapper::Mutex workerLock;//lock for access to the workers vector
	pthreadWrapper::Mutex startLock;//lock for start/stop/pause/resume operations

	class IsSameQueue {
		ReaderWriterQueue *queue;

	public:
		IsSameQueue(ReaderWriterQueue * queue) : queue(queue) {}

		bool operator () (const std::pair<ReaderWriterQueue *, std::string> &entry) {
			return queue == entry.first;
		}
	};
	
protected:
	bool isTerminatedFlag;

	/**
	 * Keeps track of a given worker started by this processor and any associated data.
	 */
	struct WorkerCache {
		Worker *processingWorker;

		/*
		 * A functor which takes QueueEntryBase& as an argument.
		 * This can be a function or it can be a
		 */
		Consumer *consumer;

		WorkerCache(Consumer *consumer): processingWorker(NULL), consumer(consumer) {}
	};

	std::vector<WorkerCache> workers;
	unsigned int nestedPauseCounter;
	unsigned int queueSize;
	SyncQueue sharedQueue;
	bool debug;

public:

	QueueProcessor(
			ProcessingQueueDataArray *dataEntries,
			unsigned int numWorkers,
			const std::vector<Consumer *> &workerConsumers,
			unsigned int queueSize) :
				numWorkers(numWorkers),
				isRunningFlag(false),
				isPausedFlag(false),
				isTerminatedFlag(false),
				nestedPauseCounter(0),
				queueSize(queueSize),
				sharedQueue(queueSize, dataEntries, &stdoutLock),
				debug(false) {
		if(numWorkers > 0 && workerConsumers.size() < 1) {
			std::cout << "no consumers provided" << endl;
			throw std::logic_error("no consumers provided");
		} else while(workers.size() < numWorkers) {
			WorkerCache worker(workerConsumers[min(workerConsumers.size() - 1, workers.size())]);
			workers.push_back(worker);
		}
	}

	virtual ~QueueProcessor();

	/*
	 * start processing the first time, or restart if paused
	 */
	bool start();

	/*
	 * pause the processing
	 * pauses existing threads
	 *
	 * if block is true, the call will block until all threads are idle
	 */
	void pause(bool block);

	/*
	 * resumes paused threads
	 */
	void resume();

	/*
	 * stop the processing:
	 * releases threads
	 * can be restarted
	 */
	void stop();

	/* terminate the processing:
	 * releases threads
	 * no restart is possible
	 */
	void terminate();

	/*
	 * returns whether it has been terminated.
	 */
	bool isTerminated() {
		return isTerminatedFlag;
	}

	/*
	 * returns whether it has been started.
	 */
	bool isRunning() {
		return isRunningFlag;
	}

	/*
	 * returns whether it has been started and is paused.
	 */
	bool isPaused() {
		return isPausedFlag;
	}

	/*
	 * signals all threads that there is work to be done.
	 */
	void broadcast();

	void writeQueueStats(std::ostream& out);

	void setDebug(bool debug);

};

}

#endif /* QUEUEPROCESSOR_H_ */
