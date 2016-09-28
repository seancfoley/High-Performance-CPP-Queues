/*
 * ProcessingQueue.h
 *
 *  Created on: Sep 7, 2010
 *      Author: sfoley
 */

#ifndef READERINDEX_H_
#define READERINDEX_H_

#include <iostream>

#include "base/Data.h"

namespace hpqueue {

/**
 * ReaderIndex contains indicators that allow queue users to navigate through a queue.
 * Depending on the queue, different fields of this object are used.
 * Whenever a queue item is removed, the worker/thread provides its own ReadIndex object,
 * which is then updated by the queue to maintain information for the next read from the queue by the same worker/thread.
 */
struct ReaderIndex {
	/* indicates the worker or thread which owns this object */
	int workerIdentifier;

	/* indicates the index of the queue to which this index object is assigned */
	int index;

	/* indicates whether the slot associated with index has been populated yet */
	volatile bool isEmpty;

	/* indicates whether the slot associated with index has been processed */
	bool isDone;

	/* indicates whether the queue is aware of this index */
	bool inQueue;

	/* these three fields allow for a linked list of these objects */
	ReaderIndex *previous;
	ReaderIndex *next;
	int nextIndex;

	ReaderIndex(int workerIdentifier) :
		workerIdentifier(workerIdentifier),
		index(-1),
		isEmpty(true),
		isDone(false),
		inQueue(false),
		previous(NULL),
		next(NULL),
		nextIndex(-1) {}

	ReaderIndex() :
		workerIdentifier(-1),
		index(-1),
		isEmpty(true),
		isDone(false),
		inQueue(false),
		previous(NULL),
		next(NULL),
		nextIndex(-1) {}

	void appendTo(std::string &str) {
		str.append(" id: ").append(Data::getStringValue(workerIdentifier)).
			append(" index: ").append(Data::getStringValue(index)).
			append(" isEmpty: ").append(Data::getStringValue(isEmpty)).
			append(" isDone: ").append(Data::getStringValue(isDone)).
			append(" inQueue: ").append(Data::getStringValue(inQueue));
	}

	friend std::ostream& operator <<(std::ostream &outputStream, ReaderIndex &readerIndex) {
		std::string str;
		readerIndex.appendTo(str);
		outputStream << str;
		return outputStream;
	}
};


}
#endif /* READERINDEX_H_ */
