/*
 * AccessEntry.h
 *
 *  Created on: Sep 14, 2010
 *      Author: sfoley
 */

#ifndef ACCESSENTRY_H_
#define ACCESSENTRY_H_

#include "base/DateTime.h"
#include "sample/SampleStatus.h"
#include "queue/QueueEntryBase.h"
#include "queue/ResultReceiverHolder.h"

namespace hpqueue {

/**
 * Holds data members for SampleQueueEntry2 to restrict access to them.
 */
struct QueueEntryData2 {
	bool withTimeStamp;
	DateTime timeStamp;
	std::string stringData;
	std::string str;
	std::vector<INT_8> bytes;

	//Data to be returned to producer by the consumer
	ResultReceiverHolder<Status> statusValue;

	QueueEntryData2(
			const std::string &stringData,
			bool withTimeStamp,
			const DateTime &timeStamp,
			const std::string &str,
			const std::vector<INT_8> &bytes,
			ResultReceiver<Status> *linkedStatus):
		withTimeStamp(withTimeStamp),
		timeStamp(timeStamp),
		stringData(stringData),
		str(str),
		bytes(bytes),
		statusValue(linkedStatus) {}

	QueueEntryData2():
		withTimeStamp(false),
		timeStamp(),
		stringData(),
		statusValue() {}

	void setStatus(const Status &status) {
		statusValue.setValue(status);
	}
};

/**
 * A sample data class.
 *
 * The data fields themselves are located in the parent class QueueEntryData1 to restrict access.
 *
 * Access is granted to classes that are friends of the access class as follows:
 * SampleQueueEntry2 &sampleEntry;
 * QueueEntryData2 &entry = Access<SampleQueueEntry2, QueueEntryData2>(sampleEntry);
 */
class SampleQueueEntry2 : private QueueEntryData2, public QueueEntryBase {
	friend class Access<SampleQueueEntry2, QueueEntryData2>;
	friend class Access<const SampleQueueEntry2, const QueueEntryData2>;

public:
	SampleQueueEntry2(
			const std::string &stringData,
			bool withTimeStamp,
			const DateTime &timeStamp,
			const string_wrapper strOrCharOrNull,
			const byte_vector_wrapper vectorOrArray,
			ResultReceiver<Status> *linkedStatus = NULL) :
		QueueEntryData2(
				stringData,
				withTimeStamp,
				timeStamp,
				strOrCharOrNull,
				vectorOrArray,
				linkedStatus) {}

	SampleQueueEntry2() {}

	virtual ~SampleQueueEntry2() {}

	QueueEntryBase& operator=(const QueueEntryBase& that) {
		const SampleQueueEntry2 *ptr = dynamic_cast<const SampleQueueEntry2 *>(&that);
		if(ptr) {
			return SampleQueueEntry2::operator=(*ptr);
		}
		return QueueEntryBase::operator=(that);
	}

	bool isNull() const {
		return false;
	}

	std::string &appendTo(std::string &str) const {
		str.append("Queue Entry 2:\n");
		appendDataTo(str);
		return str;
	}

	std::string &appendDataTo(std::string &str) const {
		str.append("\tstring data: ").append(stringData).append("\n");
		if(withTimeStamp) {
			str.append("\n\ttime stamp: ").append(timeStamp);
		}
		str.append("\tstr: ").append(str).append("\n");
		//.
		//	append("\tbytes: ").append(getStringValue(bytes)).append("\n");
		return str;
	}
};

}
#endif /* ACCESSENTRY_H_ */
