/*
 * SingleTypeDataArray.h
 *
 *  Created on: Sep 27, 2016
 *      Author: sfoley
 */

#ifndef QUEUE_SINGLETYPEDATAARRAY_H_
#define QUEUE_SINGLETYPEDATAARRAY_H_

namespace hpqueue {

template <class QueueType>
class SingleTypeDataArray : public ProcessingQueueDataArray {
	struct SingleTypeQueueData : public QueueData {
		QueueType bufferedData;

		QueueType *getQueueEntry(QueueEntryBase &entry) {
			return &bufferedData;
		}
	};

	QueueData &getEntry(unsigned int index) {
		SingleTypeQueueData *entries = static_cast<SingleTypeQueueData *>(queueDataEntries);
		return entries[index];
	}

	QueueData *resize(unsigned int queueSize) {
		return ProcessingQueueDataArray::resize(queueSize, new SingleTypeQueueData[queueSize]);
	}

	void deleteEntries(QueueData *entries) {
		SingleTypeQueueData *dataEntries = static_cast<SingleTypeQueueData *>(entries);
		delete [] dataEntries;
	}

	unsigned int getEntrySize() {
		return sizeof(SingleTypeQueueData);
	}

public:
	SingleTypeDataArray() {}

	virtual ~SingleTypeDataArray() {
		deleteEntries(queueDataEntries);
	}
};

} /* namespace hpqueue */

#endif /* QUEUE_SINGLETYPEDATAARRAY_H_ */
