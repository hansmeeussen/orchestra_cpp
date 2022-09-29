#pragma once

#include <thread>
#include <mutex>

//#include "NodeType.h"
#include "Node.h"
#include "Calculator.h"
#include "StopFlag.h"
#include "IO.h"


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
		int nrThreads = 0;
		StopFlag* sf = nullptr;
		std::mutex mtx;
		std::condition_variable condition;

		atomic_bool waitforprocessing;
	    bool processingready;
		atomic_bool quit = false;

	public:
	
		~NodeProcessor() { // Delete all the created calculators

			pleaseStop();

			for (Calculator* c : calculators) {
				delete c;
			}

			for (thread* t : threads) {
				t->join();
				delete t;
			}
		}

		NodeProcessor(Calculator*, int, StopFlag*, vector<Node*>* nodes);

		void processNodes(vector<Node*>*);

		void pleaseStop();

	private:


		Node* getNextNode();

		void runf(Calculator* c);


	};
}
