#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>
#include "Mutex.h"

namespace pthreadWrapper {

class Runnable {
	friend class Thread;

	virtual void run() = 0;

protected:
	virtual ~Runnable() {}
};

class Thread {
	pthread_t  thread;
	Runnable  *runnable;

	virtual void run() {
		runnable->run();
	}

	static void *threadEntry(void *runnable) {
		static_cast<Thread *>(runnable)->run();
		return NULL;
	}

public:
	Thread(Runnable *runnable): thread (0), runnable(runnable) {}

	virtual ~Thread() {}

	void start() {
		pthread_attr_t _attr;
		pthread_attr_init(&_attr);
		pthread_create(&thread, &_attr, threadEntry, this);
		pthread_attr_destroy(&_attr);
	}

	void join() {
		if (thread) {
			pthread_join(thread, NULL);
		}
	}
};

}

#endif /* THREAD_H_ */
