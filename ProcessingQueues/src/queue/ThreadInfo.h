/*
 * ThreadInfo.h
 *
 *  Created on: Apr 15, 2011
 *      Author: sfoley
 */

#ifndef THREADINFO_H_
#define THREADINFO_H_

#include <iomanip>
#include <sstream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

namespace hpqueue {

class ThreadInfo {
	pid_t processIdentifier;
	pid_t threadIdentifier;
	pthread_t pThread;
	bool inited;

public:
	ThreadInfo() : processIdentifier(0), threadIdentifier(0), pThread(), inited(false) {}

	virtual ~ThreadInfo() {}


	void initAsCurrentThread() {
		inited = true;
		processIdentifier = getpid();
		threadIdentifier = syscall( SYS_gettid );
		pThread = pthread_self();
	}

	void uninit() {
		inited = false;
	}

	bool isInitialized() {
		return inited;
	}

	int getThreadId() {
		return threadIdentifier;
	}

	std::string &appendDataTo(std::string &str) const {
		if(inited) {
			std::ostringstream stream;
			stream << "pid_t: " << threadIdentifier;
			return str.append(stream.str());
		}
		return str.append("identifier uninitialized");
	}

	std::string &appendTo(std::string &str) const {
		str.append("thread ");
		appendDataTo(str);
		return str;
	}

	std::string getStringValue() const {
		std::string str;
		return appendTo(str);
	}

	friend std::ostream& operator <<(std::ostream &outputStream, ThreadInfo &info) {
		std::string str;
		info.appendTo(str);
		outputStream << str;
		return outputStream;
	}
};

}

#endif /* THREADINFO_H_ */
