/*
 * QueueConstants.h
 *
 *  Created on: Mar 8, 2012
 *      Author: sfoley
 */

#ifndef QUEUECONSTANTS_H_
#define QUEUECONSTANTS_H_

namespace hpqueue {

class QueueConstants {
public:
	QueueConstants() {}

	virtual ~QueueConstants() {}

	/*
	 * Constants indicating status of an entry relative to the queue, indicating why an entry is not on the queue.
	 * Some of these are returned from the add member functions of queues.
	 */
	const static int IS_FULL;
	const static int CANNOT_ADD;
	const static int IS_TERMINATED;
	const static int DEQUEUED;
};

}

#endif /* QUEUECONSTANTS_H_ */
