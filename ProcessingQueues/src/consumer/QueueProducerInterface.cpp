/*
 * OperationsProcessor.cpp
 *
 *  Created on: Sep 15, 2011
 *      Author: sfoley
 */

#include "QueueProducerInterface.h"

namespace hpqueue {

struct ProcessorQueueAdder {
	SyncWriterQueue &queue;
	QueueEntryBase &entry;

	ProcessorQueueAdder(SyncWriterQueue &queue, QueueEntryBase &entry) : queue(queue), entry(entry) {}

	virtual int add() {
		return queue.add(entry);
	}

	void resize() {
		queue.resize(queue.getNewQueueSize());
	}

	bool isMaxSize() {
		return queue.getCurrentSize() >= MAX_QUEUE_SIZE(queue.getEntrySize());
	}

	virtual ~ProcessorQueueAdder(){}
};

void QueueProducerInterface::resumeThreadsForResize(ResizeControls &controls) {
	resume(); //resume workers
	controls.resumeAdders();
}

void QueueProducerInterface::pauseThreadsForResize(ResizeControls &controls) {
	/*
	 * The pauseAdders call can block while an adding thread continues trying to add, waiting for space to appear.
	 *
	 * Therefore we cannot pause workers first.
	 *
	 * Anyway, in general it makes sense to continue emptying the queue until the last possible moment.
	 */
	controls.pauseAdders();
	pause(true); //wait for workers to pause
}

void QueueProducerInterface::add(QueueEntryBase &entry) {
	ProcessorQueueAdder adder(sharedQueue, entry);
	int index = addToQueue(adder, resizeControls);
	if(index >= 0) {
		broadcast();
	}
}

int QueueProducerInterface::addToQueue(ProcessorQueueAdder &adder, ResizeControls &controls) {
	if(isTerminatedFlag) {
		return QueueConstants::IS_TERMINATED;
	}
	controls.waitForResize(); /* check resize flag to see if a resize in progress, if so, then wait */
	int index = adder.add(); /* try to add */
	while(checkResizable(index, adder) /* check if queue full, if so, handle the case where no more resizing is allowed: wait until space available */
			&& controls.checkFull(index) /* check if queue full, if so, grab resize lock */
			&& controls.checkFullForResize(index = adder.add()) /* try again and check if queue still full, if so, set resize flag to begin resize */
			) {
		pauseThreadsForResize(controls); /* ensure no threads are adding and also pause worker threads */
		adder.resize(); /* make the queue bigger */
		resumeThreadsForResize(controls); /* end the resize by resetting the resize flag and resume workers */
		index = adder.add(); /* try again */
	}
	return index;
}

/**
 * Continue trying to add when the queue cannot be resized.
 */
bool QueueProducerInterface::checkResizable(int &index, ProcessorQueueAdder &adder) {
	if(index == QueueConstants::IS_FULL && adder.isMaxSize()) {
		do {
			::usleep(100);
			index = adder.add();
		} while (index == QueueConstants::IS_FULL);
	}
	return true;
}

}
