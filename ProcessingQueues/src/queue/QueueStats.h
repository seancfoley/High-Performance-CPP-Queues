/*
 * QueueStats.h
 *
 *  Created on: Nov 16, 2011
 *      Author: sfoley
 */

#ifndef QUEUESTATS_H_
#define QUEUESTATS_H_

#include <cstdio>
#include <iostream>

#include "threading/Mutex.h"

namespace hpqueue {

class QueueStats {
	/* the current capacity */
	unsigned int size;

	/* total entries added to the queue */
	unsigned int addedCount;

	/* entries previously handled and no longer in the queue */
	unsigned int removedCount;

	pthreadWrapper::Mutex outLock;

public:
	QueueStats(unsigned int size) :
		size(size),
		addedCount(0),
		removedCount(0) {}

	inline void incrementAddedCount() {
		addedCount++;
	}

	/**
	 * Increment the number of items that have been removed from the queue to which these stats pertain
	 */
	inline void incrementRemovedCount(unsigned int increment) {
		/*
		 * we need a synchronization here because this is called by client threads to update the queue stats,
		 * but it is also called by the worker threads themselves, when they die, to make their final update.
		 */
		__sync_fetch_and_add(&removedCount, increment);
	}

	inline void incrementRemovedCount() {
		removedCount++;
	}

	void setSize(unsigned int newSize) {
		size = newSize;
	}

	bool isUsed() {
		return addedCount > 0;
	}

	void print(FILE *fp, const std::string &queueName) {
		outLock.acquire();
		fprintf(fp, "enqueued (handled) size: %d (%d) %d %s\n", addedCount - removedCount, removedCount, size, queueName.c_str());
		outLock.release();
	}

	void print(std::ostream& dout, const std::string &queueName) {
		outLock.acquire();
		dout << "enqueued (handled) size: " << addedCount - removedCount << " (" << removedCount << ") " << size  << " " << queueName << std::endl;
		outLock.release();
	}
};

}

#endif /* QUEUESTATS_H_ */
