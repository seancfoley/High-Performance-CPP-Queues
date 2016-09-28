/*
 * DateTime.h
 *
 *  Created on: Dec 22, 2010
 *      Author: sfoley
 */

#ifndef DATETIME_H_
#define DATETIME_H_

#include <string>
#include <iostream>
#include <sstream>
#include <cstring>
#include "base/Data.h"

namespace hpqueue {

#define DATE_TIME_STRFTIME_STRPTIME_FORMAT "%Y-%m-%d %H:%M:%S"

class DateTime {
	time_t timeSecs; //number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC 
	 /* in the future, if we want to support microseconds in timestamp, use	 
	 * UINT_64 microSecs;
	 */

public:
	DateTime(): timeSecs(0) {}

	DateTime(time_t dateTime) :	timeSecs(dateTime) {} 

	DateTime(const DateTime &dateTime){
		this->timeSecs = dateTime.timeSecs;
	}

	DateTime(const std::string dateTime) {
		struct tm brokenDownTime;
		strptime(dateTime.c_str(), "%Y-%m-%d %H:%M:%S", &brokenDownTime);
		timeSecs = mktime(&brokenDownTime);
	}


	virtual ~DateTime() {}

	void add(const DateTime &dateTime) {
		add(dateTime.timeSecs);
	}

	void add(time_t otherTime) {
		this->timeSecs += otherTime;
	}

	operator time_t() const {
		return timeSecs;
	}

	void clear() {
		timeSecs = 0;
	}

	bool hasTime() const {
		return (timeSecs > 0);
	}

	// only use this operator when it is necessary
	operator std::string () const {
		char timeChars[50];
		struct tm timeValues;
		localtime_r(&timeSecs, &timeValues);
		strftime(timeChars, sizeof(timeChars), DATE_TIME_STRFTIME_STRPTIME_FORMAT, &timeValues);
		return std::string(timeChars);
	}

	DateTime& operator=(const DateTime& that) {
		timeSecs = that.timeSecs;
		return *this;
	}

	friend std::istream& operator >>(std::istream &inputStream, DateTime &data) {
		inputStream >> data.timeSecs;
		return inputStream;
	}

	friend std::ostream& operator <<(std::ostream &outputStream, const DateTime &data) {
		outputStream << data.timeSecs;
		return outputStream;
	}
};

}

#endif /* DATETIME_H_ */
