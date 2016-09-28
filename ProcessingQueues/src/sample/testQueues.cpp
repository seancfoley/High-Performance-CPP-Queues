#include <iostream>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>

#include "consumer/QueueProducerInterface.h"
#include "queue/ResultReceiver.h"
#include "SampleQueueEntryConsumer.h"

using namespace std;
using namespace hpqueue;
using namespace pthreadWrapper;

template <typename T>
void print(vector<T> &values) {
	ostringstream stream;
	copy(values.begin(), values.end(), ostream_iterator<T>(stream, ", "));
	string out = stream.str();
	cout << "[" << ((out.size() >= 2) ? out.erase(out.size() - 2) : "") << "]" << endl;
}

Mutex counterMutex;
int counter = 0;

int getNext() {
	counterMutex.acquire();
	int result = ++counter;
	counterMutex.release();
	return result;
}

int count1 = 7, count2 = 41, count3 = 3;

void testPopulate(QueueProducerInterface &operationsProcessor) {
	//first we add more than the queue size
	//this will check that it resizes ok with no threads
	ThreadInfo threadInfo;
	threadInfo.initAsCurrentThread();
	int i = 1;
	for(; i <= count1; i++) {
		time_t currentTime = time(NULL);
		stringstream stream;
		stream << "thread " << threadInfo.getThreadId() << " " << i << " total " << getNext() << " " << currentTime;
		SampleQueueEntry1 data(
				i,
				stream.str(),
				4,
				DateTime(34567));
		operationsProcessor.add(data);
	}

	operationsProcessor.writeQueueStats(cout);
	
	//then we start it
	operationsProcessor.start();
	
	i = 1;
	//then we add some more, quickly
	for(; i <= count2; i++) {
		time_t currentTime = time(NULL);
		stringstream stream;
		stream << "thread " << threadInfo.getThreadId() << " " << i << " total " << getNext() << " " << currentTime;
		SampleQueueEntry1 data(
				i,
				stream.str(),
				5,
				DateTime(676868));
		operationsProcessor.add(data);

		SampleQueueEntry2 data2(
				stream.str(),
				true,
				DateTime(11),
				"xy",
				"yz");
		operationsProcessor.add(data2);

	}

	i = 1;
	//for these we will block to receive results
	for(; i <= count3; i++) {
		time_t currentTime = time(NULL);
		stringstream stream;
		stream << "thread " << threadInfo.getThreadId() << " " << i << " total " << getNext() << " " << currentTime;
		ResultReceiver<Status> statusHolder1;
		ResultReceiver<Status> *statusHolderPtr = &statusHolder1;

		SampleQueueEntry1 data(
				i,
				stream.str(),
				4,
				DateTime(34567),
				statusHolderPtr);
		operationsProcessor.add(data);
		Status status = statusHolder1.getValue();
		//cout << "result was " << status << endl;
	}

	operationsProcessor.writeQueueStats(cout);
}

class QueueAdder: public Runnable {
	QueueProducerInterface *queueProcessor;

	void run() {
		testPopulate(*queueProcessor);
	}
public:
	QueueAdder(QueueProducerInterface *queueProcessor) :
		queueProcessor(queueProcessor) {}
};

int adderCount = 5;

void testQueuesMultipleThreads(QueueProducerInterface &operationsProcessor) {
	int numThreads = adderCount;
	vector<Thread *> threads;
	QueueAdder adder(&operationsProcessor);
	for(int i=0; i<numThreads; i++) {
		Thread *thread = new Thread(&adder);
		threads.push_back(thread);
	}
	for(int i=0; i<numThreads; i++) {
		threads[i]->start();
	}
	for(int i=0; i<numThreads; i++) {
		threads[i]->join();
	}
	for(int i=0; i<numThreads; i++) {
		delete threads[i];
	}
}

int max(int a, int b) {
	return (a > b) ? a : b;
}

int main() {
	cout << "Starting " << endl;
	int numWorkerThreads = 8;
	int numProcessors = 5;
	vector<QueueProducerInterface *> processors;
	vector<ProcessingQueueDataArray *> dataArrays;
	vector<vector<SampleQueueEntryConsumer *> > allConsumers;
	for(int i = 0; i < numProcessors; i++) {
		vector<SampleQueueEntryConsumer *> sampleConsumers;
		for(int j = 0; j < numProcessors; j++) {
			sampleConsumers.push_back(new SampleQueueEntryConsumer);
		}
		vector<Consumer *> consumers;
		copy(sampleConsumers.begin(), sampleConsumers.end(), std::back_inserter(consumers));
		dataArrays.push_back(new SampleDataArray);
		processors.push_back(new QueueProducerInterface(dataArrays.back(), numWorkerThreads, consumers, 3 /* queue size */));
		allConsumers.push_back(sampleConsumers);
	}
	//for(int i=0; i<numProcessors; i++) {
	//	processors[i]->setDebug(true);
	//}
	for(int i=0; i<numProcessors; i++) {
		testQueuesMultipleThreads(*processors[i]);

		/* see how many of the entries have been consumed, until we have seen all of them */
		UINT_32 totalExpected = adderCount * (count1 + (2 * count2) + count3);
		cout << "counting the " << totalExpected << " entries consumed" << endl;
		int rounds = 0;
		while(totalExpected > 0 && rounds++ < 3) {
			sleep(1);//not possible to know how long it will take, but let's not start counting before the consumers had a chance to finish
			vector<SampleQueueEntryConsumer *> consumers = allConsumers[i];
			for(int j = consumers.size() - 1; j >= 0; j--) {
				SampleQueueEntryConsumer *consumer = consumers[j];
				UINT_32 *ptr = &consumer->count;
				UINT_32 removed = __sync_lock_test_and_set(ptr, 0);
				totalExpected -= removed;
				cout << "consumer " << (j + 1) << " handled " << removed << ", remaining down to " << totalExpected << endl;
			}
		}
		if(totalExpected > 0) {
			cout << "missed " << totalExpected << endl;
		}
		processors[i]->writeQueueStats(cout);
		cout << "Terminating processor " << processors[i] << endl << endl;
		processors[i]->terminate();
	}

	for(int i=0; i<numProcessors; i++) {
		delete processors[i];
		delete dataArrays[i];
	}

	cout << endl << "Ending " << endl;
	return 0;
}
