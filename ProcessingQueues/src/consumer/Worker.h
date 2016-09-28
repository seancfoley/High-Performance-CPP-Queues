/*
 *  Created on: Sep 20, 2010
 *      Author: sfoley
 */

#ifndef CONSUMER_H_
#define CONSUMER_H_

#include <sstream>

#include "threading/Condition.h"
#include "threading/Thread.h"

namespace hpqueue {

class Worker : public pthreadWrapper::Runnable {
	volatile bool isStartedFlag;
	volatile bool isPausedFlag;
	volatile bool isIdleFlag;
	volatile bool isTerminatedFlag;
	volatile bool isDeadFlag;
	volatile bool isWaitingFlag;

	pthreadWrapper::Mutex startLock;

	pthreadWrapper::Mutex isWorkLock;
	pthreadWrapper::Condition isWorkWaiting;
	volatile bool haveWorkJoiners;

	pthreadWrapper::Mutex hasDiedLock;
	pthreadWrapper::Condition hasDied;

	pthreadWrapper::Mutex hasPausedLock;
	pthreadWrapper::Condition hasPaused;
	pthreadWrapper::Condition isIdle;

	/* implement these virtual methods in a subclass */

	/* called by the worker itself when it starts up */
	virtual void init() = 0;

	/* called by the worker itself when it is about to terminate */
	virtual void finalize() = 0;

	/* does a bit of work when there is work to do, returns true if there may be more work to do */
	virtual bool doWork() = 0;

	/* return true when there is work to do */
	virtual bool isWork() = 0;

	void run();

	void signalDead();

protected:

	std::string name;

	int identifier;

	bool debug;

private:
	pthreadWrapper::Thread thread;

	unsigned int timeoutSeconds;

public:

	Worker(int identifier, const char *name, unsigned int timeoutSeconds = 0) :
		isStartedFlag(false),
		isPausedFlag(false),
		isIdleFlag(false),
		isTerminatedFlag(false),
		isDeadFlag(false),
		isWaitingFlag(false),
		haveWorkJoiners(false),
		name(getName(identifier, name)),
		identifier(identifier),
		debug(false),
		thread(this),
		timeoutSeconds(timeoutSeconds) {}

	virtual ~Worker() {}

	static std::string getName(int identifier, const char *name) {
		const char *prefix = "Consumer ";
		if(name) {
			return std::string(prefix).append(name);
		}
		std::ostringstream stream;
		stream << prefix << (identifier + 1);
		return stream.str();
	}

	/*
	 * Start the worker.
	 * A worker can be started at most once.
	 * If called for the first time after stop() has been
	 * called, it will have no effect, the worker cannot be started once it is terminated.
	 */
	void start();

	/*
	 * Terminate the worker.
	 */
	void stop();

	/*
	 * Pause the worker.  If called before start, worker will pause when started,
	 * but the worker will call init first.
	 * Has no effect if called after stop() has been called.
	 * If block is true, the call will block until the pause has commenced.
	 */
	void pause(bool block);

	/*
	 * Resume a paused worker.
	 */
	void resume();

	/*
	 * Suspend execution of the calling thread until the worker is dead.
	 */
	void join();

	/*
	 * Signal the worker that there is work to do.
	 *
	 * If block is true, then the call will block until the worker has completed a call to doWork().
	 * If the worker has been paused, this may take a long time.
	 */
	void signal(bool block = false);

	/* The worker is dead. */
	bool isDead();

	/* start() has been called. */
	bool isStarted();

	/* stop() has been called and the worker has been terminated.  Worker might not yet be dead though. */
	bool isTerminated();

	/* Worker is paused. */
	bool isPaused();

	virtual void setDebug(bool debug);

	virtual void updateStats() {}

	const std::string &getName() const {
		return name;
	}
};

}

#endif /* CONSUMER_H_ */
