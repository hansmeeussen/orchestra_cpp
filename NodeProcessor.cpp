#include "NodeProcessor.h"

namespace orchestracpp
{
	NodeProcessor::NodeProcessor(Calculator* calculator, int nrThreads, StopFlag* sf, vector<Node*>* nodes) {
		this->nodes = nodes;
		this->sf = sf;

		if (nrThreads > 0) { // if nr of threads is negative we determine number automatically
			this->nrThreads = nrThreads;
		} else {
		    this->nrThreads = std::thread::hardware_concurrency(); // make number of threads equal to number of logical processors
	    }

		if (this->nrThreads > nodes->size()) {
			this->nrThreads = nodes->size();
		}

		cout << "Nr threads: "<<this->nrThreads << endl;
		// create independent calculator copies, one for each thread.

		for (int n = 0; n < this->nrThreads; n++) {
			cout << "creating calculator " << n << endl;
			Calculator* tmpCalculator = calculator->clone();

			// perform a first calculation on an equilibrated node
			// this is useful for benchmarking different methods, as the first calculation
			// for each calculator is slow because of initialisation
			bool success = tmpCalculator->calculate(nodes->at(0), sf);
			cout << "first calculation was successful " << success << endl;


			calculators.push_back(tmpCalculator);
		}

		waitforprocessing = true;

		// set up and start the threads
		for (int n = 0; n < this->nrThreads; n++) {
			// documentation https://thispointer.com/c11-start-thread-by-member-function-with-arguments/
			threads.push_back(new std::thread(&NodeProcessor::runf, this, calculators.at(n)));
		}

	}

	void NodeProcessor::processNodes(vector<Node*>* nodes, int mo) {
		this->memoryOption = mo;
		processNodes(nodes);
	}

	void NodeProcessor::processNodes(vector<Node*>* nodes) {

		this->nodes = nodes;

		//notify threads that they can start processing
		{
			unique_lock<mutex> lck(mtx);
			currentNodeNr = 0;
			waitforprocessing = false;
			processingready = false;
			condition.notify_all();
		}

    	// wait here until processing is ready
		{
			unique_lock<mutex> lck(mtx);
			condition.wait(lck, [this] {return processingready; });

		}

	}



	int NodeProcessor::partition(vector<Node*>* nodes, int low, int high) {
		// we take the element at high index as pivot
		double  pivot = nodes->at(high)->getvalue(sort_indx);

		int i = low; // Index of smaller element 

		for (int j = low; j <= high - 1; j++) {
			// we swap all elements that are smaller than the pivot
			if (nodes->at(j)->getvalue(sort_indx) < pivot) {
				swap(i, j, nodes);
				i++; 
			}
		}
		swap(i, high, nodes);
		return (i);
	}

	void NodeProcessor::quickSort(vector<Node*>* nodes, int low, int high) {
		if (low < high) {
			int pi = partition(nodes, low, high);
			quickSort(nodes, low, pi - 1);
			quickSort(nodes, pi + 1, high);
		}
	}

	void NodeProcessor::swap(int from, int to, vector<Node*>* nodes) {
		Node* tmp = nodes->at(from);
		nodes->at(from) = nodes->at(to);
		nodes->at(to) = tmp;
	}

	void NodeProcessor::sortNodes(vector<Node*>* nodes, string variableName) {
		sort_indx = nodes->at(0)->nodeType->index(variableName);
		quickSort(nodes, 0, nodes->size() - 1);
	}

	//we return nullPtr when all nodes are processed
	Node* NodeProcessor::getNextNode() {

		// if one of the treads has processed the last node and has set the processingready flag
		// the other threads that enter this function can stop as well
		

		std::lock_guard<mutex> lock(mtx); // use a lock to allow only synchronized access (one thread at the time)
		                                  // this lock is automatically released when it gets out of scope

		// if one of the treads has processed the last node and has set the processingready flag
		// the other threads that enter this function can stop as well
		if (processingready)return nullptr;

		if (currentNodeNr < nodes->size()) {
			Node* node = (*nodes)[currentNodeNr];
			currentNodeNr++;
			return node;
		} else {
			processingready = true;
			condition.notify_one(); // notify the waiting main thread
			return  nullptr;
		}
	}

	void NodeProcessor::pleaseStop() {
		std::lock_guard<mutex> lock(mtx);
		quit = true;
		waitforprocessing = false;
		condition.notify_all();
	}

	// the function that is called by the independent threads
	// each thread uses its own calculator copy
	void NodeProcessor::runf(Calculator* c) {

		while (true) {

			{
				// this is the place where the threads wait until notified to start processing
				unique_lock<mutex> lck(mtx);
				condition.wait(lck, [this] {return !waitforprocessing; });
			}

			// quit if asked to do so (from nodeprocessor destructor)
			if (quit)break;

			// here the thread will ask for a node and calculate this
			// until all nodes are processed
			while (true) {
				Node* node = getNextNode();
				if (node == nullptr)break;
				if (memoryOption == 0) {
			    	c->calculate(node, sf);
			    }else{
				    c->calculate2(node, sf);
			    }
			}
		}

	}
}


