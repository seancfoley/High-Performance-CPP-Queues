/*
 * ResultReceiverHolder.h
 *
 *  Created on: Oct 3, 2010
 *      Author: sfoley
 */

#ifndef RESULTRECEIVERHOLDER_H_
#define RESULTRECEIVERHOLDER_H_

#include "threading/Mutex.h"
#include "ResultReceiver.h"

namespace hpqueue {

/**
 * A holder for a result receiver object intended to receive a result produced by consuming a queue element.
 *
 */
template <typename T>
class ResultReceiverHolder {
	pthreadWrapper::Mutex identifierLock;
	ResultReceiver<T> *resultReceiver;

public:
	ResultReceiverHolder(ResultReceiver<T> *resultReceiver = NULL):
		resultReceiver(resultReceiver) {}

	virtual ~ResultReceiverHolder() {}

	/*
	 * Returns whether the holder is linked
	 */
	bool isLinked() {
		return resultReceiver != NULL;
	}

	/* called by the consumer thread handling the entry in the queue or an associated thread */
	void setEmpty() {
		if(resultReceiver != NULL) {
			identifierLock.acquire();
			if(resultReceiver != NULL) {
				resultReceiver->setPopulated();
				resultReceiver = NULL;
			}
			identifierLock.release();
		}
	}

	/* called by the consumer thread handling the entry in the queue or an associated thread */
	void setValue(T value) {
		if(resultReceiver != NULL) {
			identifierLock.acquire();
			if(resultReceiver != NULL) {
				resultReceiver->setValue(value);
				resultReceiver->setPopulated();
				resultReceiver = NULL;
			}
			identifierLock.release();
		}
	}
};

}

#endif /* RESULTRECEIVERHOLDER_H_ */
