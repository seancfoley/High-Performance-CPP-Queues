/*
 * ConsumingWorker.cpp
 *
 *  Created on: Sep 22, 2010
 *      Author: sfoley
 */

#include <unistd.h>

#include "QueueConsumerWorker.h"

using namespace std;

namespace hpqueue {

void QueueConsumerWorker::init() {
	queueAccess.startAccess();
}

void QueueConsumerWorker::finalize() {
	queueAccess.endAccess();
	updateStats();
}

bool QueueConsumerWorker::doWork() {
	QueueEntryBase &entry = queueAccess.remove();
	if(!entry.isNull()) {
		queueAccess.incrementRemovedCount();
		consumer->handle(entry);
		return true;
	}
	return false;
}

bool QueueConsumerWorker::isWork() {
	return !queueAccess.isEmpty();
}

void QueueConsumerWorker::setDebug(bool debug) {
	Worker::setDebug(debug);
	queueAccess.queue->setDebug(debug);
}

void QueueConsumerWorker::updateStats() {
	queueAccess.updateStats();
}

}
