
#include <stddef.h>

#include "SampleQueueEntry1.h"
#include "SampleQueueEntry2.h"
#include "queue/ProcessingQueueDataArray.h"

using namespace std;

namespace hpqueue {

class SampleDataArray : public ProcessingQueueDataArray {
	struct SampleQueueData : public QueueData {
		SampleQueueEntry1 data1; /* will hold both LiveSessionEntry and SessionEntry */
		SampleQueueEntry2 data2;

		QueueEntryBase *getQueueEntry(QueueEntryBase &entry) {
			QueueEntryBase *queueEntry;
			if(dynamic_cast<SampleQueueEntry1 *>(&entry)) {
				queueEntry = &data1;
			} else if(dynamic_cast<SampleQueueEntry2 *>(&entry)) {
				queueEntry = &data2;
			} else {
				queueEntry = NULL;
			}
			return queueEntry;
		}
	};

	bool isCompatibleEntry(QueueEntryBase &entry) {
		return true;
	}

	QueueData &getEntry(unsigned int index) {
		SampleQueueData *entries = static_cast<SampleQueueData *>(queueDataEntries);
		return entries[index];
	}

	QueueData *resize(unsigned int queueSize) {
		return ProcessingQueueDataArray::resize(queueSize, new SampleQueueData[queueSize]);
	}

	void deleteEntries(QueueData *entries) {
		SampleQueueData *dataEntries = static_cast<SampleQueueData *>(entries);
		delete [] dataEntries;
	}

	unsigned int getEntrySize() {
		return sizeof(SampleQueueData);
	}

public:
	SampleDataArray() {}

	virtual ~SampleDataArray() {
		deleteEntries(queueDataEntries);
	}
};

}
