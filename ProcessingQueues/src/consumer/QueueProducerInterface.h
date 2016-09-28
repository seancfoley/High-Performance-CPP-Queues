/*
 * OperationsProcessor.h
 *
 *  Created on: Sep 15, 2011
 *      Author: sfoley
 */

#ifndef QUEUEPRODUCERINTERFACE_H_
#define QUEUEPRODUCERINTERFACE_H_

#include <vector>

#include "QueueConsumerWorker.h"
#include "QueueProcessor.h"
#include "queue/ResizeControls.h"

namespace hpqueue {

/* performs the queue-access operations for adding to a queue */
struct ProcessorQueueAdder;

/**
 * The external interface to the queue processor, with the methods for adding and manipulating the queue.
 */
class QueueProducerInterface : public QueueProcessor {

	int addToQueue(ProcessorQueueAdder &adder, ResizeControls &controls);

	/*
	 * ResizeControls are used for resizing queues, coordinating a resize operation amongst several threads.
	 */
	ResizeControls resizeControls;

	void resumeThreadsForResize(ResizeControls &controls);

	void pauseThreadsForResize(ResizeControls &controls);

	static bool checkResizable(int &index, ProcessorQueueAdder &adder);

public:
	/*
	 * Functor is a function or type which takes QueueEntryBase& as an argument.
	 *
	 * These will be the user-supplied consumers of queue entries, one for each running worker thread.
	 *
	 * If this processor is stopped and restarted, then the worker thread assigned to each functor may change.
	 *
	 * If there are fewer functors than workers, the last functor will be shared amongst the remaining workers.
	 * So you can specify just a single functor if desired.
	 */
	QueueProducerInterface(
			ProcessingQueueDataArray *dataEntries,
			unsigned int numWorkers,
			const std::vector<Consumer *> &workerConsumers,
			unsigned int queueSize): QueueProcessor(dataEntries, numWorkers, workerConsumers, queueSize) {}

	virtual ~QueueProducerInterface() {}

	void add(QueueEntryBase &entry);

	void writeQueueStats(std::ostream& out) {
		QueueProcessor::writeQueueStats(out);
	}
};

}

#endif /* QUEUEPRODUCERINTERFACE_H_ */
