/*
 * ProcessingQueue.cpp
 *
 *  Created on: Nov 20, 2011
 *      Author: sfoley
 */

#include "ProcessingQueue.h"

using namespace std;

namespace hpqueue {

unsigned int ProcessingQueue::getNewQueueSize() {
	unsigned int newSize = getCurrentSize();
	if(newSize >= UINT_MAX / 2) {
		newSize = UINT_MAX;
	} else {
		newSize *= 2;
	}

	unsigned int maxSize = MAX_QUEUE_SIZE(getEntrySize());
	if(newSize > maxSize) {
		return maxSize;
	}
	return newSize;
}

}

