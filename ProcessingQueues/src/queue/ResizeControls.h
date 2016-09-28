/*
 * ResizeControls.h
 *
 *  Created on: Nov 7, 2011
 *      Author: sfoley
 */

#ifndef RESIZECONTROLS_H_
#define RESIZECONTROLS_H_

#include "threading/Condition.h"

namespace hpqueue {

/**
 * Queues cannot be resized while being accessed by reader or writer threads.
 *
 * ResizeControls coordinates resize operations amongst several threads.
 * It ensures that only one resize is attempted at a time,
 * and that all queue-accessing threads are blocked when the resizing operation takes place.
 *
 * All threads that are coordinated must use the same ResizeControls object.
 *
 * Resizing procedure:
 *
 * 1. waitForResize(): An incoming thread checks to see if a resizing is in progress by checking the "resize" boolean. If so, it waits.
 *					Afterwards, it increments the counter "addingCount", which indicates queue access is in progress.
 *
 * 2. Then (outside this object) it attempts to add to the queue, and decrements addingCount afterwards.
 *
 * 3. checkFull(int): If the queue was full, it checks to see if a resizing is in progress (again).  If so, it waits.  If not, it's done.
 *
 * 4. checkFullForResize(int): Afterwards, it attempts to add again, while holding the resize lock that is acquired in checkFull.
 * Once that is done, it calls checkFullForResize, continuing to hold the resize lock,
 * and if the add failed again, it will become the resizing thread.  If the add succeeded, it's done.
 * If it is attempting a resize, it will set the resize boolean to indicate it is resizing.
 *
 * By holding the lock while calling checkFullForResize, only one thread will attempt a resize at a time,
 * and only immediately after failing to add to the queue with no intervening resize (by a different thread) taking place since then.
 *
 * 5. At this point it must wait for all queue-accessing threads to pause, both other threads adding,
 * and also reading threads.  Newly adding threads will pause because the resize boolean is now true,
 * and threads in the midst of adding will have paused when addingCount returns to 0.
 *
 * 6. Then the queue is resized.
 *
 * 7. resumeAdders(): Afterwards, all threads are resumed, addingCount is incremented again to indicate the thread will
 * attempt queue access again, and the process repeats itself from 2.
 *
 */
class ResizeControls {
	pthreadWrapper::Mutex resizeQueuesLock;

	pthreadWrapper::Condition isResizingCond;

	bool resizing;

	int addingCount;

public:
	ResizeControls() : resizing(false), addingCount(0) {}

	virtual ~ResizeControls() {}

	/*
	 * If a queue resize is in progress, will block until that resizing is complete.
	 *
	 * Before exiting it increments addingCount to indicate to other threads that queue access is in progress by the caller.
	 */
	void waitForResize() {
		resizeQueuesLock.acquire();
		while(resizing) {
			isResizingCond.wait(resizeQueuesLock);
		}
		addingCount++;
		resizeQueuesLock.release();
	}

	/*
	 * Checks if the queue was full.
	 *
	 * If so, it will wait until it has possession of the resizing lock,
	 * and will exit this function holding the lock, returning true.
	 *
	 * If the queue was not full, it will return false and will exit not holding the resizing lock.
	 *
	 * It will decrement addingCount, therefore any further queue access can be done only if holding the resizing lock.
	 */
	bool checkFull(int index) {
		bool isFull = (index == ReaderWriterQueue::IS_FULL);
		resizeQueuesLock.acquire();
		addingCount--;
		if(isFull) {
			while(resizing) {
				isResizingCond.wait(resizeQueuesLock);
			}
			/* when full, we intentionally do not release the lock */
		} else {
			resizeQueuesLock.release();
		}
		return isFull;
	}

	/*
	 * Expected to be called while holding the resizing lock.
	 * Checks if the queue was full.
	 * If so, it will set the resizing flag while holding the lock,
	 * so that a queue resizing can be initiated by the caller.
	 * It will not be holding the resizing lock when the function exits.
	 */
	bool checkFullForResize(int index) {
		bool isFull = (index == ReaderWriterQueue::IS_FULL);
		resizing = isFull; //pause other adders
		resizeQueuesLock.release();
		return isFull;
	}

	/**
	 * When pauseAdders is called, the resizing boolean is true so that incoming threads will be blocked from queue access.
	 *
	 * However, there may be queue access already in progress.
	 *
	 * This function blocks until all queue-accessing threads have completed their access.
	 */
	void pauseAdders() {
		while(addingCount) {//wait for other adders to pause
			usleep(100);
		}
	}

	/**
	 * Sets the resizing boolean to false, and resumes any threads currently blocked from queue access.
	 *
	 * It also increments addingCount to indicate that a queue access will be attempted by the caller.
	 */
	void resumeAdders() {
		resizeQueuesLock.acquire();
		addingCount++;
		resizing = false;
		isResizingCond.broadcast(); //resume adders
		resizeQueuesLock.release();
	}
};

}

#endif /* RESIZECONTROLS_H_ */
