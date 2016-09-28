/*
 * SampleQueueEntryConsumer.h
 *
 *  Created on: Sep 26, 2016
 *      Author: sfoley
 */

#ifndef SAMPLE_SAMPLEQUEUEENTRYCONSUMER_H_
#define SAMPLE_SAMPLEQUEUEENTRYCONSUMER_H_

#include "threading/Condition.h"
#include "SampleQueueEntry1.h"
#include "SampleQueueEntry2.h"
#include "consumer/Consumer.h"

namespace hpqueue {

class SampleQueueEntryConsumer: public Consumer {

	/* we are using one of these consumers per worker, so no need for thread-safety */

	void handle(QueueEntryBase &entry) {
		SampleQueueEntry1 *sampleQueueEntry1;
		SampleQueueEntry2 *sampleQueueEntry2;
		if((sampleQueueEntry1 = dynamic_cast<SampleQueueEntry1 *>(&entry))) {
			QueueEntryData1 &data = Access<SampleQueueEntry1, QueueEntryData1>(*sampleQueueEntry1);
			data.setStatus(Status(Status::STATUS_SUCCESS));
			count++;
		} else if((sampleQueueEntry2 = dynamic_cast<SampleQueueEntry2 *>(&entry))) {
			QueueEntryData2 &data = Access<SampleQueueEntry2, QueueEntryData2>(*sampleQueueEntry2);
			data.setStatus(Status(Status::STATUS_SUCCESS));
			count++;
		}
	}

public:
	UINT_32 count;

	SampleQueueEntryConsumer(): count(0) {}

	~SampleQueueEntryConsumer() {}
};

}


#endif /* SAMPLE_SAMPLEQUEUEENTRYCONSUMER_H_ */
