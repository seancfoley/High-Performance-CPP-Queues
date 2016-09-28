/*
 * QueueEntryBase.h
 *
 *  Created on: Sep 9, 2010
 *      Author: sfoley
 */

#ifndef QUEUEENTRYBASE_H_
#define QUEUEENTRYBASE_H_

#include <iostream>
#include <vector>
#include "base/Access.h"
#include "base/Data.h"

namespace hpqueue {

/*
 * Represents an entry in a queue
 */
class QueueEntryBase: public Data {
public:
	QueueEntryBase() {}

	virtual ~QueueEntryBase() {}

	static QueueEntryBase nullEntry;

	virtual std::string &appendDataTo(std::string &str) const {
		return str;
	}

	virtual std::string &appendTo(std::string &str) const {
		return appendDataTo(str);
	}

	using Data::getStringValue;

	std::string getStringValue() {
		std::string str;
		return appendTo(str);
	}

	friend std::ostream& operator <<(std::ostream &outputStream, QueueEntryBase &data) {
		std::string str;
		data.appendTo(str);
		outputStream << str;
		return outputStream;
	}

	/**
	 * override this method if a memcpy works as well as using the assignment operator.
	 * For a memcpy to work, the entry should hold plain data.
	 */
	void copyTo(QueueEntryBase *destination) {
		*destination = *this;
	}

	virtual bool isNull() const {
		return true;
	}

	virtual QueueEntryBase& operator=(const QueueEntryBase& that) {
		Data::operator=(that);
		return *this;
	}
};

}

#endif /* QUEUEENTRYBASE_H_ */
