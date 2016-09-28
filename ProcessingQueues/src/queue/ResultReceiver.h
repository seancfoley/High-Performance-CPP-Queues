/*
 * ResultReceiver.h
 *
 *  Created on: Oct 4, 2010
 *      Author: sfoley
 */

#ifndef RESULTRECEIVER_H_
#define RESULTRECEIVER_H_

#include "threading/Condition.h"
#include "base/DataHolder.h"

namespace hpqueue {

template <typename T>
class ResultReceiver: public DataHolder<T> {

	pthreadWrapper::Mutex isPopulatedLock;
	pthreadWrapper::Condition isPopulatedCond;
	bool isPopulatedFlag;

public:
	
	/* called by the consumer thread handling the entry in the queue or an associated thread */
	void setPopulated() {
		isPopulatedLock.acquire();
		isPopulatedFlag = true;
		isPopulatedCond.signal();
		isPopulatedLock.release();
	}

	/**
	 * This function will wait for the returned value to be populated by consumers before returning.
	 */
	T &getValue(bool block) {
		if(block && !isPopulatedFlag) {
			isPopulatedLock.acquire();
			if(!isPopulatedFlag) {
				isPopulatedCond.wait(isPopulatedLock);
			}
			isPopulatedLock.release();
		}
		return getCurrentValue();
	}

public:
	ResultReceiver(): isPopulatedFlag(false) {}

	virtual ~ResultReceiver() {}

	/**
	 * This method will not block to wait for the data to be populated
	 * by another thread.
	 */
	T &getCurrentValue() {
		return DataHolder<T>::getCurrentValue();
	}

	/**
	 * This method will block, waiting for the data to be populated by a consumer thread
	 */
	T &getValue() {
		return DataHolder<T>::getValue();
	}
};

}

#endif /* RESULTRECEIVER_H_ */
