#pragma once

#include <condition_variable>
#include <thread>
#include <mutex>
//#include <experimental/random>

//#include "NodeType.h"
#include "Node.h"
#include "Calculator.h"
#include "StopFlag.h"
#include "IO.h"

//#include <random>
//#include <ctime>

using namespace std;
using namespace orchestracpp;
namespace orchestracpp
{

	class NodeProcessor {
	

	private:
		vector<Calculator*> calculators;
		std::vector<thread*> threads;
		vector<Node*>* nodes = nullptr;
		int currentNodeNr = 0;

		StopFlag* sf = nullptr;

		std::mutex mtx;
		std::condition_variable condition;
		std::condition_variable busyCondition;
		int nrBusyThreads = 0;

		bool quit = false;
		bool startProcessing = false;
		bool lastNodeTaken = false;

		int memoryOption = 0; // 0 = no memory, so use previous node value (transport),  
		                      // 1 = use previous succesful node  (serial calculation, transport intialize)

		//int sort_indx;

		int setSize = 1;

	public:

		int nrThreads = 0;

		~NodeProcessor() { // Delete all the created calculators

			pleaseStop();

			// stop and delete the threads before deleting the calculators
			for (thread* t : threads) {
				t->join();
				delete t;
			}

			for (Calculator* c : calculators) {
				delete c;
			}

		}

		NodeProcessor(Calculator*, int, StopFlag*, vector<Node*>* nodes);

		void processNodes(vector<Node*>*);

		void processNodes(vector<Node*>*, int memoryOption);

		void pleaseStop();

		void processNodesSingleThread(vector<Node*>* nodes, int mo);

	private:

	//	Node* getNextNode();

		vector<Node*>* getNextNodes();

		void runf(Calculator* c);

		void incNrBusy();

		void decNrBusy();

		bool isReady();

	};
}
