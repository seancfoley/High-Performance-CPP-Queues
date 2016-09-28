/*
 * QueueProcessor.cpp
 *
 *  Created on: Sep 7, 2010
 *      Author: sfoley
 */

#include <cstdio>
#include <vector>
#include <string>
#include <utility>

#include "QueueConsumerWorker.h"
#include "QueueProcessor.h"

using namespace std;

namespace hpqueue {

QueueProcessor::~QueueProcessor() {
	terminate();
}

bool QueueProcessor::start() {
	if(isTerminatedFlag) {
		return true;
	}
	startLock.acquire();
	bool result = true;
	if(!isTerminatedFlag) {
		if(isRunningFlag) {
			/* try to start any threads that could not start previously */
			workerLock.acquire();
			for(unsigned int i=0; i<numWorkers; i++) {
				WorkerCache &worker = workers[i];
				worker.processingWorker->start();
			}
			workerLock.release();
		} else {
			isRunningFlag = true;
			workerLock.acquire();
			workers.reserve(numWorkers);
			unsigned int i=0;
			for(; i<numWorkers; i++) {
				Worker *processingWorker = new QueueConsumerWorker(
						i,
						&sharedQueue,
						numWorkers,
						workers[i].consumer,
						&stdoutLock);
				workers[i].processingWorker = processingWorker;
				processingWorker->setDebug(this->debug);
				processingWorker->start();
			}
			workerLock.release();
		}
	}
	startLock.release();
	return result;
}

void QueueProcessor::stop() {
	if(!isRunningFlag) {
		return;
	}

	startLock.acquire();

	if(isRunningFlag) {
		workerLock.acquire();

		/*
		 * Trigger each worker to do work until it has none left.
		 *
		 * Otherwise work can be lost, or even repeated when the processor is restarted.
		 *
		 * If workers are paused, this will block until they are resumed.
		 *
		 * We will wait until the worker threads are done with their work.
		 */
		vector<WorkerCache>::iterator it;
		for (it = workers.begin(); it != workers.end(); it++) {
			WorkerCache &worker = *it;
			Worker *processingWorker = worker.processingWorker;
			processingWorker->signal(true);
		}

		/* now tell the workers to stop */
		for (it = workers.begin(); it != workers.end(); it++) {
			WorkerCache &worker = *it;
			Worker *processingWorker = worker.processingWorker;
			processingWorker->stop();
		}

		/* wait for the workers to stop */
		for (it = workers.begin(); it != workers.end(); it++) {
			WorkerCache &worker = *it;
			Worker *processingWorker = worker.processingWorker;
			worker.processingWorker = NULL;
			processingWorker->join();
			delete processingWorker;
		}

		workerLock.release();
	}


	isRunningFlag = isPausedFlag = false;
	startLock.release();
}

void QueueProcessor::broadcast() {
	workerLock.acquire();
	for (vector<WorkerCache>::iterator it = workers.begin(); it != workers.end(); it++) {
		WorkerCache &worker = *it;
		Worker *processingWorker = worker.processingWorker;
		if(processingWorker) {
			processingWorker->signal();
		}
		//TODO can we call isWork() on the worker just before we wake it up?
		//if no work, do we need to signal all further workers?

		//However, keep in mind that for the multi-reader queue, just because one doesn't have work that doesn't mean they all don't
		//in fact, since they form a queue (SynReaderList) it would be best to wake them up in the queue order

		//This change is not trivial... we would need to take a long look
	}
	workerLock.release();
}

void QueueProcessor::terminate() {
	if(isTerminatedFlag) {
		return;
	}

	/* setting is Terminated ensures no restart possible after this */
	isTerminatedFlag = true;

	stop();

	workerLock.acquire();
	workers.clear();
	workerLock.release();
}

void QueueProcessor::pause(bool block) {
	if(isTerminatedFlag) {
		return;
	}
	startLock.acquire();
	if(!isPausedFlag) {
		if(isRunningFlag) {
			workerLock.acquire();
			vector<WorkerCache>::iterator it;
			for (it = workers.begin(); it != workers.end(); it++) {
				WorkerCache &worker = *it;
				worker.processingWorker->pause(false);
			}
			if(block) {
				for (it = workers.begin(); it != workers.end(); it++) {
					WorkerCache &worker = *it;
					worker.processingWorker->pause(true);
				}
			}
			workerLock.release();
		}
		isPausedFlag = true;
	}
	nestedPauseCounter++;
	startLock.release();
}

void QueueProcessor::resume() {
	if(isTerminatedFlag) {
		return;
	}
	startLock.acquire();
	if(isPausedFlag) {
		nestedPauseCounter--;
		if(!nestedPauseCounter) {
			workerLock.acquire();
			if(isRunningFlag) {
				vector<WorkerCache>::iterator it;
				for (it = workers.begin(); it != workers.end(); it++) {
					WorkerCache &worker = *it;
					worker.processingWorker->resume();
				}
			}
			workerLock.release();
			isPausedFlag = false;
		}
	}
	startLock.release();
}

void QueueProcessor::writeQueueStats(ostream& out) {
	workerLock.acquire();
	for (vector<WorkerCache>::iterator it = workers.begin(); it != workers.end(); it++) {
		WorkerCache &worker = *it;
		Worker *processingWorker = worker.processingWorker;
		if(processingWorker) {
			processingWorker->updateStats();
		}
	}
	workerLock.release();

	/*
	 * Queue writing stats are being constantly updated by queue writers.
	 * Queue reading stats are updated only when we call updateCounts on each worker (above).
	 *
	 * Here we get a snapshot of the stats for each shared queue at this moment.
	 *
	 * Due to the ordering of these calls, the write counts might be more recent than the read counts,
	 * so the queue's contents may appear larger.
	 *
	 * If we moved these two calls to get a snapshot of global queue stats above the above loop, we would have the
	 * opposite effect.
	 */
	QueueStats stats = sharedQueue.getStats();

	stats.print(out, "queue");
}

void QueueProcessor::setDebug(bool debug) {
	this->debug = debug;
	workerLock.acquire();
	for (vector<WorkerCache>::iterator it = workers.begin(); it != workers.end(); it++) {
		WorkerCache &worker = *it;
		Worker *processingWorker = worker.processingWorker;
		if(processingWorker) {
			processingWorker->setDebug(debug);
		}
	}
	workerLock.release();
}

}


