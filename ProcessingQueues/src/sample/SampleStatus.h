/*
 * Status.h
 *
 *  Created on: Dec 2, 2010
 *      Author: sfoley
 */

#ifndef STATUS_H_
#define STATUS_H_

#include <string>

namespace hpqueue {

/**
 * Holds the status of an operation.
 */
class Status {
public:
	enum statusCode {
		STATUS_SUCCESS = 0,
		STATUS_ERROR,
		STATUS_UNKNOWN
	} status;

	Status(): status(STATUS_UNKNOWN) {}

	Status(enum statusCode status) : status(status) {}

	virtual ~Status() {}

	std::string &appendDataTo(std::string &str) const {
		str.append("\tresult: ").append(isSuccess() ? "success" : "failure");
		return str;
	}

	std::string &appendTo(std::string &str) const {
		str.append("Operation Status:\n");
		appendDataTo(str);
		return str;
	}

	bool isSuccess() const {
		return status == STATUS_SUCCESS;
	}

	bool isError() const {
		return status == STATUS_ERROR;
	}

	friend std::ostream& operator <<(std::ostream &outputStream, const Status &data) {
		std::string str;
		data.appendTo(str);
		outputStream << str;
		return outputStream;
	}
};

}

#endif /* STATUS_H_ */
