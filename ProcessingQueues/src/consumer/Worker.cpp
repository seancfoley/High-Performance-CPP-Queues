/*
 * Worker.cpp
 *
 *  Created on: Sep 20, 2010
 *      Author: sfoley
 */


#include <iostream>

#include "consumer/Worker.h"

using namespace std;

namespace hpqueue {

void Worker::start() {
	if(!isStartedFlag && !isTerminatedFlag) {
		startLock.acquire();
		if(!isStartedFlag && !isTerminatedFlag) {
			isStartedFlag = true;
			thread.start();
		}
		startLock.release();
	}
}

void Worker::run() {
	cout << "started worker " << name << endl;
	init();
	try {
		do {
			if(isPausedFlag) {
				hasPausedLock.acquire();
				if(isPausedFlag && !isTerminatedFlag) {
					isIdleFlag = true;
					isIdle.signal();
					hasPaused.wait(hasPausedLock);
					isIdleFlag = false;
				}
				hasPausedLock.release();
			} else {
				if(!doWork()) {
					isWorkLock.acquire();
					if(!isWork() && !isTerminatedFlag && !isPausedFlag) {
						isWaitingFlag = true;
						if(haveWorkJoiners) {
							isWorkWaiting.broadcast();
						}
						if(timeoutSeconds) {
							isWorkWaiting.wait(isWorkLock, timeoutSeconds * 1000000);
						} else {
							isWorkWaiting.wait(isWorkLock);
						}
						isWaitingFlag = false;
					}
					isWorkLock.release();
				}
			}
		} while(!isTerminatedFlag);
	} catch(...) {
		cout << "error in worker " << name << ", " << "terminating." << endl;

		isTerminatedFlag = true;

		/* release any threads that may be waiting for this worker to pause or to wait for work */

		hasPausedLock.acquire();
		if(isPausedFlag) {
			isIdle.signal();
			isIdleFlag = false;
		}
		hasPausedLock.release();

		isWorkLock.acquire();
		if(haveWorkJoiners) {
			isWorkWaiting.broadcast();
		}
		isWaitingFlag = false;
		isWorkLock.release();
	}
	finalize();
	signalDead();
}

void Worker::signalDead() {
	hasDiedLock.acquire();
	isDeadFlag = true;
	hasDied.signal();
	hasDiedLock.release();
}

void Worker::join() {
	if(!isDeadFlag) {
		hasDiedLock.acquire();
		while(!isDeadFlag) {
			hasDied.wait(hasDiedLock);
		}
		hasDiedLock.release();
	}
}

void Worker::signal(bool block) {
	isWorkLock.acquire();
	if(isWaitingFlag) {
		isWorkWaiting.signal();
	}
	if(block && !isTerminatedFlag && isStartedFlag) {
		haveWorkJoiners = true;
		isWorkWaiting.wait(isWorkLock);
	}
	isWorkLock.release();
}

void Worker::pause(bool block) {
	if(!isTerminatedFlag) {
		hasPausedLock.acquire();
		if(!isTerminatedFlag) {
			if(!isPausedFlag) {
				isPausedFlag = true;
				
				/*
				 * Ensure we are not waiting for work, but instead we're waiting on the paused condition.
				 *
				 * Be careful here, we are acquiring isWorkLock after acquiring hasPausedLock,
				 * so we can never do the reverse order.
				 */
				signal();
			}
			if(block && !isIdleFlag && isStartedFlag) {
				isIdle.wait(hasPausedLock);
			}
		}
		hasPausedLock.release();
	}
}

void Worker::resume() {
	if(isPausedFlag) {
		hasPausedLock.acquire();
		if(isPausedFlag) {
			isPausedFlag = false;
			hasPaused.signal();
		}
		hasPausedLock.release();
	}
}

void Worker::stop() {
	/* once isTerminated is true, the worker will not wait for work or pause */
	isTerminatedFlag = true;
	bool started = isStartedFlag;
	if(!started) {
		startLock.acquire();
		started = isStartedFlag;
		startLock.release();
	}
	if(started) {
		/*
		 * Once isTerminated is true, the worker will not signal or pause,
		 * but we must ensure the worker was not already paused or waiting.
		 */
		resume();

		signal();

		/*
		 * Release any threads waiting for a pause
		 */
		hasPausedLock.acquire();
		isIdle.signal();
		isIdleFlag = false;
		hasPausedLock.release();
	} else {
		/*
		 * Reset the setPaused flag.
		 * The worker will never start, so we must be the one to release any threads waiting for a pause.
		 */
		hasPausedLock.acquire();
		isPausedFlag = false;
		isIdle.signal();
		isIdleFlag = false;
		hasPausedLock.release();

		/* The worker will never start, so we must be the one to release any joined threads */
		signalDead();
	}
}

bool Worker::isDead() {
	return isDeadFlag;
}

bool Worker::isTerminated() {
	return isTerminatedFlag;
}

bool Worker::isStarted() {
	return isStartedFlag;
}

bool Worker::isPaused() {
	return isPausedFlag;
}

void Worker::setDebug(bool debug) {
	this->debug = debug;
}

}

