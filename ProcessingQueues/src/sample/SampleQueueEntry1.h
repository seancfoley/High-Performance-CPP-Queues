/*
 * SentenceEntry.h
 *
 *  Created on: Nov 16, 2010
 *      Author: sfoley
 */

#ifndef SENTENCEENTRY_H_
#define SENTENCEENTRY_H_

#include "SampleStatus.h"
#include "queue/QueueEntryBase.h"
#include "base/DateTime.h"
#include "queue/ResultReceiverHolder.h"

namespace hpqueue {

/**
 * Holds data members for SampleQueueEntry1 to restrict access to them.
 */
struct QueueEntryData1 {
	//Data to be returned to producer by the consumer
	ResultReceiverHolder<Status> statusHolder;

	UINT_64 id1;
	std::string string1;
	INT_32 id2;

	bool withTimeStamp;
	DateTime timeStamp;

	QueueEntryData1(
		UINT_64 id1,
		const std::string &string1,
		INT_32 id2,
		const DateTime &timeStamp,
		ResultReceiver<Status> *linkedStatus):
			statusHolder(linkedStatus),
			id1(id1),
			string1(string1),
			id2(id2),
			withTimeStamp(timeStamp.hasTime()),
			timeStamp(timeStamp) {}

	QueueEntryData1():
		statusHolder(),
		id1(0),
		string1(),
		id2(0),
		withTimeStamp(false),
		timeStamp() {}

	void setStatus(const Status &status) {
		statusHolder.setValue(status);
	}
};

/**
 * A sample data class.
 *
 * The data fields themselves are located in the parent class QueueEntryData1 to restrict access.
 *
 * Access is granted to classes that are friends of the access class as follows:
 * SampleQueueEntry1 &sampleEntry;
 * QueueEntryData1 &entry = Access<SampleQueueEntry1, QueueEntryData1>(sampleEntry);
 */
class SampleQueueEntry1: private QueueEntryData1, public QueueEntryBase {
	friend class Access<SampleQueueEntry1, QueueEntryData1>;
	friend class Access<const SampleQueueEntry1, const QueueEntryData1>;

public:
	SampleQueueEntry1(
			UINT_64 id1,
			const std::string &string1,
			INT_32 id2,
			const DateTime &timeStamp,
			ResultReceiver<Status> *linkedStatus = NULL) :
				QueueEntryData1(
						id1,
						string1,
						id2,
						timeStamp,
						linkedStatus) {}

	SampleQueueEntry1() {}

	virtual ~SampleQueueEntry1() {}

	bool isNull() const {
		return false;
	}

	QueueEntryBase& operator=(const QueueEntryBase& that){
		const SampleQueueEntry1 *ptr = dynamic_cast<const SampleQueueEntry1 *>(&that);
		if(ptr) {
			return SampleQueueEntry1::operator=(*ptr);
		}
		return QueueEntryBase::operator=(that);
	}

	std::string &appendDataTo(std::string &str) const {
		str.append("\tidentifier 1: ").append(getStringValue(id1)).
			append("\n\tstring 1: ").append(string1).
			append("\n\tidentifier 2: ").append(getStringValue(id2));
		if(withTimeStamp) {
			str.append("\n\ttime stamp: ").append(timeStamp);
		}
		return str;
	}

	std::string &appendTo(std::string &str) const {
		str.append("Sentence Entry:\n");
		appendDataTo(str);
		return str;
	}
};

}

#endif /* SENTENCEENTRY_H_ */
