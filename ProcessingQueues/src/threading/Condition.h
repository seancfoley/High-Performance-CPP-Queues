#ifndef CONDITION_H_
#define CONDITION_H_

#include <sys/time.h>

#include "Mutex.h"

namespace pthreadWrapper {

class Condition {
	pthread_cond_t condition;

  public:
	Condition() {
		pthread_cond_init(&condition, NULL);
	}

	~Condition() {
		pthread_cond_destroy(&condition);
	}

	void signal() {
		pthread_cond_signal(&condition);
	}

	void broadcast() {
		pthread_cond_broadcast(&condition);
	}

	int wait(Mutex &mutex) {
		return pthread_cond_wait(&condition, &mutex.mutex);
	}

	int wait(Mutex &mutex, unsigned long nsec) {
		struct timeval now;
		struct timespec	timeout;
		gettimeofday(&now, NULL);
		timeout.tv_sec =  now.tv_sec + nsec / 1000000;
		timeout.tv_nsec = now.tv_usec * 1000 + nsec % 1000000;
		return pthread_cond_timedwait(&condition, &mutex.mutex, &timeout);
	}
};

}

#endif /* CONDITION_H_ */
