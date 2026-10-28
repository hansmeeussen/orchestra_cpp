#pragma once

#include <condition_variable>
#include <thread>
#include <mutex>
#include <stack>

//#include "NodeType.h"
#include "Node.h"
#include "Calculator.h"
#include "StopFlag.h"
#include "IO.h"

using namespace std;
using namespace orchestracpp;
namespace orchestracpp
{

	class NodeProcessor final {
	

	private:
		vector<Calculator*> calculators;
		vector<thread*> threads;
		vector<Node*>* nodes = nullptr;
		int currentNodeNr = 0;

		StopFlag* stopFlag = nullptr;

		std::mutex mtx; 
	
		bool quit = false;

		std::condition_variable readyForProcessingCondition;
		bool readyForProcessing = false;
		int nrThreadsWaitingForProcessing = 0;

		std::condition_variable finishedProcessingCondition;
		bool finishedProcessing = false;
		int nrThreadsFinishedProcessing = 0;

		int memoryOption = 0; // 0 = no memory, so use previous node value (transport),  
		                      // 1 = use previous succesful node  (serial calculation, transport intialize)
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

		vector<Node*>* getNextNodes();

		void runf(Calculator* c);

	};
}
