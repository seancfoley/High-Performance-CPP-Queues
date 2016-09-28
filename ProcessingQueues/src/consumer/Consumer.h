/*
 * Consumer.h
 *
 *  Created on: Sep 26, 2016
 *      Author: sfoley
 */

#ifndef CONSUMER_CONSUMER_H_
#define CONSUMER_CONSUMER_H_

#include "queue/QueueEntryBase.h"

namespace hpqueue {

class Consumer {
	friend class QueueConsumerWorker;

protected:
	virtual void handle(QueueEntryBase &entry) = 0;

	Consumer() {}

	virtual ~Consumer() {}
};

} /* namespace hpqueue */

#endif /* CONSUMER_CONSUMER_H_ */
