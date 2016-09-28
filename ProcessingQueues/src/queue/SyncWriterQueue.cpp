/*
 * SyncWriterQueue.cpp
 *
 *  Created on: Sep 23, 2010
 *      Author: sfoley
 */

#include "SyncWriterQueue.h"

namespace hpqueue {

int SyncWriterQueue::add(QueueEntryBase &entry) {
	addMutex.acquire();
	int index = ReaderWriterQueue::add(entry);
	addMutex.release();
	return index;
}

}
