/*
 * SyncReaderList.cpp
 *
 *  Created on: Sep 30, 2010
 *      Author: sfoley
 */

#include <stdexcept>

#include "SyncReaderList.h"
#include "ThreadInfo.h"


using namespace std;

namespace hpqueue {

void SyncReaderList::moveOut(ReaderIndex &index) {
	if (&index == back) {
		back = index.next;
		/*
		 * Since we're at the back, we must adjust the queue's read index to indicate the last slot being read.
		 *
		 * The queue's read index is always one above the last slot being read/used.
		 * When a thread is finished using that slot, the read index is adjusted here.
		 * Even if that slot is now empty, you cannot increase the read index above it, because that would
		 * ruin the queue's empty check (isEmpty when readIndex == writeIndex, isFull when readIndex == writeIndex + 1).
		 * When the queue is empty, it would think it's full.  readIndex must stay at the same level or below writeIndex.
		 */
		queue.readIndex = back->index;
	} else {
		index.previous->next = index.next;
	}
	index.next->previous = index.previous;
}

bool SyncReaderList::moveToFront(ReaderIndex &index) {
	if(!index.inQueue) {
		throw std::logic_error("must be added to queue ReadList to use SyncReaderQueue");
	}
	bool result = true;
	indexMutex.acquire();

	if(debug) {
		ThreadInfo threadInfo;
		threadInfo.initAsCurrentThread();
		stdoutLock->acquire();
		cout << threadInfo.getThreadId() << " start moveToFront with front " << front << " and back " << back << " and my read index " << &index << " " << *this << endl;
		stdoutLock->release();
	}

	/* try to move to the front so we can read another slot */
	int nextIndex = queue.nextIndex(front->index);
	if(nextIndex != back->index) {
		/* we can move to the front without hitting the back */
		index.index = nextIndex;
		if(&index != front) {
			moveOut(index);
			index.previous = front;
			index.next = NULL;
			front = front->next = &index;
		} else if (&index == back) {
			queue.readIndex = index.index;    //we are at front and back.
		}
	} else {
		/*
		 * The list spans the whole queue (unlikely to ever happen with larger queues)
		 */
		/*
		 * Even if we cannot jump to a slot to read, we can (and must) free up at least one slot to be available
		 * for writing if we are at the back of the reader list.  This ensures that when the reader list spans the whole queue, that we still free up space
		 * for writing (and hence we no longer span the whole queue).  If we cannot read a new slot we must free up space anyway.
		 *
		 */
		if(&index == back) {
			if(debug) {
				ThreadInfo threadInfo;
				threadInfo.initAsCurrentThread();
				stdoutLock->acquire();
				cout << threadInfo.getThreadId() << " start squeeze with front " << front << " and back " << back << " and read index " << queue.readIndex << " and write index " << queue.writeIndex << " and my read index " << &index << " " << *this << endl;
				stdoutLock->release();
			}

			if(index.next != front) {
				back = index.next;
				back->previous = NULL;
				back->next->previous = back;
				index.next = front;
				index.previous = front->previous;
				front->previous->next = &index;
				front->previous = &index;

				index.index = index.previous->index;
			} /* else there is only two entries in the list */
			queue.readIndex = back->index;

			if(debug) {
				ThreadInfo threadInfo;
				threadInfo.initAsCurrentThread();
				stdoutLock->acquire();
				cout << threadInfo.getThreadId() << " end squeeze with front " << front << " and back " << back << " and read index " << queue.readIndex << " and write index " << queue.writeIndex << " and my read index " << &index << " " << *this << endl;
				stdoutLock->release();
			}
		}
		result = false;
	}
	if(debug) {
		ThreadInfo threadInfo;
		threadInfo.initAsCurrentThread();
		stdoutLock->acquire();
		cout << threadInfo.getThreadId() << " end moveToFront " << *this  << endl;
		stdoutLock->release();
	}
	indexMutex.release();
	if(!result) {
		/*
		 * We yield here because we want someone else to run, since there is nothing for this thread to read.
		 * We yield to either a queue writer if we added new space for writing, or to a different reader.
		 */
		sched_yield();
	}
	return result;
}

void SyncReaderList::remove(ReaderIndex &index) {
	if(!index.inQueue) {
		return;
	}
	indexMutex.acquire();

	if(debug) {
		ThreadInfo threadInfo;
		threadInfo.initAsCurrentThread();
		stdoutLock->acquire();
		cout << threadInfo.getThreadId() << " removing " << index << " with front " << *front << " and back " << *back << " and read index " << queue.readIndex << " and write index " << queue.writeIndex << " " << *this << endl;
		stdoutLock->release();
	}

	if(&index != front) {
		if(!index.isDone) {
			/*
			 * Waiting to read from our slot.
			 * Adjust the remaining readers.
			 * We will move the front to our slot, and the slot previous to the front will become the front,
			 */
			if(index.next == front) {
				/* the front will remain the front, we are just moving it back one slot */
				front->previous = index.previous;
				if(&index != back) {
					front->previous->next = front;
				} else {
					back = front;
				}
				front->index = index.index;
			} else {
				/* the front will replace us, and the second-to-front will become the new front */
				ReaderIndex *newFront = front->previous;
				newFront->next = NULL;

				front->previous = index.previous;

				if(&index != back) {
					front->previous->next = front;
				} else {
					//new back
					back = front;
				}
				front->next = index.next;
				front->next->previous = front;
				front->index = index.index;
				front = newFront;
			}
		} else {
			if (&index == back) {
				back = index.next;
			} else {
				index.previous->next = index.next;
			}
			index.next->previous = index.previous;
		}
		index.previous = index.next = NULL;
	} else if(&index == back) {
		/* we are both front and back, so we are the only entry in the queue, and the entry is done */
		if(index.isDone && !queue.isEmpty(index)) {
			queue.readIndex = queue.nextIndex(index.index);
		}
		front = back = index.previous = index.next = NULL;
	} else {
		//at front of queue, not at back
		front = index.previous;
		front->next = index.previous = index.next = NULL;
	}
	if(debug) {
		ThreadInfo threadInfo;
		threadInfo.initAsCurrentThread();
		stdoutLock->acquire();
		cout << threadInfo.getThreadId() << " removed " << index << " and read index " << queue.readIndex << " and write index " << queue.writeIndex << " " << *this << endl;
		stdoutLock->release();
	}
	indexMutex.release();
	index.isEmpty = false;
	index.inQueue = false;
	index.isDone = false;
}

void SyncReaderList::add(ReaderIndex &index) {
	if(index.inQueue) {
		throw std::logic_error("already added");
	}
	indexMutex.acquire();
	if(front == NULL) {
		index.index = queue.readIndex;
		front = back = index.next = index.previous = &index;
		index.isDone = false;
	} else {
		int nextIndex = queue.nextIndex(front->index);
		if(nextIndex != back->index) {
			index.next = NULL;
			index.previous = front;
			index.index = nextIndex;
			front = front->next = &index;
			index.isDone = false;
		} else {
			index.next = front;
			index.previous = front->previous;
			front->previous = &index;
			index.previous->next = &index;
			index.index = index.previous->index;
			index.isDone = true;
		}
	}
	indexMutex.release();
	index.isEmpty = true;
	index.inQueue = true;
}

void SyncReaderList::adjust(int adjustment, int queueReadIndex) {
	ReaderIndex *readerIndex = back;
	while(readerIndex != NULL) {
		int index = readerIndex->index;
		bool slotIsPopulated = queue.adjustIndexForComparison(index, queueReadIndex) <
				queue.adjustIndexForComparison(queue.writeIndex, queueReadIndex);
		/*
		 *  if we point to a populated slot that has been moved,
		 *  or we point to a non-populated slot that has been relocated,
		 *  then we adjust the index.
		 */
		if(slotIsPopulated ? (index > queue.writeIndex) : (index < queue.writeIndex)) {
			readerIndex->index += adjustment;
		}
		readerIndex = readerIndex->next;
		if(readerIndex == back) {
			break;
		}
	}
}

void SyncReaderList::setDebug(bool debug) {
	this->debug = debug;
}

std::ostream& operator <<(std::ostream &outputStream, SyncReaderList &readerList) {
	readerList.indexMutex.acquire();
	ReaderIndex *readerIndex = readerList.back;
	bool first = true;
	int i = 0;
	while(readerIndex != NULL) {
		outputStream << endl << i++;
		if(first) {
			outputStream << "    ";
		} else {
			outputStream << " -> ";
		}
		outputStream  << "[" << *readerIndex << "]";
		readerIndex = readerIndex->next;
		if(readerIndex == readerList.back) {
			break;
		}
		first = false;
	}
	outputStream << endl;
	outputStream  << " front: ";
	if(readerList.front) {
		outputStream  <<  *readerList.front;
	} else {
		outputStream  <<  "NULL";
	}
	outputStream  << endl;
	readerList.indexMutex.release();
	return outputStream;
}

}
