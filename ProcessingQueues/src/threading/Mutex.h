#ifndef MUTEX_H_
#define MUTEX_H_

#include <pthread.h>

namespace pthreadWrapper {

class Mutex {
	friend class Condition;
	mutable pthread_mutex_t mutex;

	public:
	Mutex(int type = PTHREAD_MUTEX_RECURSIVE) {
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, type);
		pthread_mutex_init(&mutex, &attr);
		pthread_mutexattr_destroy(&attr);
	}

	void acquire() {
		pthread_mutex_lock(&mutex);
	}

	void release() {
		pthread_mutex_unlock(&mutex);
	}
	
	~Mutex() {
		pthread_mutex_destroy(&mutex);
	}
};

}


#endif /* MUTEX_H_ */
