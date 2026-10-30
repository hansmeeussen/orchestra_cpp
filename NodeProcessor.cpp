#include "NodeProcessor.h"


namespace orchestracpp
{
	NodeProcessor::NodeProcessor(Calculator* calculator, int nrThreads_in, StopFlag* sf, vector<Node*>* nodes) {
		this->nodes = nodes;
		this->stopFlag = sf;

		if (nrThreads_in > 0) { // if nr of threads is negative we determine number automatically
			this->nrThreads = nrThreads_in;
		} else {
		    this->nrThreads = std::thread::hardware_concurrency(); // make number of threads equal to number of logical processors
	    }


		// we do not want to have more threads than nodes
		if (this->nrThreads > nodes->size()) {
			this->nrThreads = nodes->size();
		}

		

		cout << "Nr threads: "<< this->nrThreads << "  " << nrThreads_in << endl;

		// create independent calculator copies, one for each thread.
		for (int n = 0; n < this->nrThreads; n++) {
			cout << "creating calculator " << n << endl;
			Calculator* tmpCalculator = calculator->clone();

			// perform a first calculation on an equilibrated node
			// this is useful for benchmarking different methods, as the first calculation
			// for each calculator is slow because of initialisation
			bool success = tmpCalculator->calculate(nodes->at(0), sf);
			cout << "first calculation was successful " << endl;

			calculators.push_back(tmpCalculator);
		}

		// set up and start the threads
		for (int n = 0; n < this->nrThreads; n++) {
			// each thread has its own independent calculator, so these can run in parallel
			threads.push_back(new std::thread(&NodeProcessor::runf, this, calculators.at(n)));
		}
	}

	// The setSize determines the number of nodes that is processed in one go by each thread. 
	// The optimal setsize is larger when nodes that are related and calculation times per node are similar
	// memoryOption == 0  // use information present in node as start estimation. Efficient for using results from previous timestep.
	// memoryOption == 1  // use information from previous calculation by calculator as start estimation. Efficient for related nodes.
	void NodeProcessor::processNodes(vector<Node*>* nodes, int mo) {
		this->memoryOption = mo;

		if (memoryOption == 1) {
			setSize = nodes->size() / (nrThreads);
		}
		else {
			setSize = nodes->size() / (nrThreads * 4);
		}

		if (setSize < 1) setSize = 1; 

		processNodes(nodes);
	}

	void NodeProcessor::processNodes(vector<Node*>* nodes) {

		this->nodes = nodes;

		if (nrThreads == 1) {
			processNodesSingleThread(nodes, memoryOption);
			return;
		}

		if (memoryOption == 1) {
			// Nodes are related, so we do first a single calculation with node 0,
			// and use results to update the lastSuccessful node of all the calculators in the threads
			calculators.at(0)->calculate(nodes->at(0), stopFlag);
			for (Calculator* c : calculators) {
				if (c->lastSuccessfulNode2 != nullptr) {
					c->lastSuccessfulNode2->clone(nodes->at(0));
				}
			}
		}

		// initialize the flag variables and notify threads that they can start processing
		{
			unique_lock<mutex> lock(mtx);

			currentNodeNr = 0;
			nrThreadsWaitingForProcessing++;

			// we wait until all worker threads plus this main thread are waiting at this barrier
			if (nrThreadsWaitingForProcessing == nrThreads + 1) {
				readyForProcessing = true; // signal that the worker threads can pass the barrier and can start processing
				// initialize the variables at the start of the processing task
				nrThreadsFinishedProcessing = 0; 
				finishedProcessing = false;
				// and notify the other worker threads and main thread
				readyForProcessingCondition.notify_all();
			}
			readyForProcessingCondition.wait(lock, [this] {return readyForProcessing; }); // true indicates that we can (re)start processing
	    }

		// Here the processing by the the threads happens
		// and we wait here until this is ready

		{
			unique_lock<mutex> lock(mtx);
			nrThreadsFinishedProcessing++;

			if (nrThreadsFinishedProcessing == nrThreads + 1) {
				finishedProcessing = true;
				nrThreadsWaitingForProcessing = 0;
				readyForProcessing = false;
				finishedProcessingCondition.notify_all();
			}
			finishedProcessingCondition.wait(lock, [this] {return finishedProcessing; }); // true indicates that processing is ready, so we can continue
		}

		// Processing ready. This method returns, but 
		// the worker threads are waiting at the the startprocessing barrier

	}

	/* This method returns a vector with a set of node (pointers) to be calculated 
	   When ready a nullptr is returned
	*/
	vector<Node*>* NodeProcessor::getNextNodes() {		
		// we return either a vector of nodes, or null indicating that we are ready
		int start;
		int number;

		{
			std::lock_guard<mutex> lock(mtx);
			start = currentNodeNr;
			number = nodes->size() - start;
			if (number < 1) return nullptr;
			if (number > setSize) number = setSize;
			currentNodeNr += number;
		}

		vector<Node*>* ntbc = new vector<Node*>;

		for (int n = 0; n < number; n++) {
    		ntbc->push_back((*nodes)[start + n]);
		}

		return ntbc;
	}

	void NodeProcessor::pleaseStop() {
		std::lock_guard<mutex> lock(mtx);
		quit = true;
		readyForProcessingCondition.notify_all();
	}

	
	// This version processes a set of nodes at the time
	void NodeProcessor::runf(Calculator* c) {

		while (true) {

			{
				// this is the place where the threads wait until notified to start processing
				unique_lock<mutex> lock(mtx);

				nrThreadsWaitingForProcessing++;

				if (nrThreadsWaitingForProcessing == nrThreads + 1) {
					readyForProcessing = true;
					nrThreadsFinishedProcessing = 0;
					finishedProcessing = false;
					readyForProcessingCondition.notify_all();
				}
				readyForProcessingCondition.wait(lock, [this] {return readyForProcessing; }); // true indicates that we can restart processing

				if (quit) { break; }

	    	}

			// here the thread will ask for a (set of) node(s) and calculate this
			// until all nodes are processed
			while (true) {
				vector<Node*>* ntbc = getNextNodes();
				if (ntbc == nullptr) {
					break; // we leave the calculation loop and go to the readyprocessing barrier
				}

				if (memoryOption == 0) { // no memory, simply use current node as start estimation	
				    for (int n = 0; n < ntbc->size(); n++) {
						c->calculate(ntbc->at(n), stopFlag);
					}
				}
				else {
					for (int n = 0; n < ntbc->size(); n++) { // use latest calculated node as start estimation
						c->calculate2(ntbc->at(n), stopFlag);
					}
				}

				delete(ntbc);
			}

			// here we wait until all threads are ready
			{
				unique_lock<mutex> lock(mtx);
	
				nrThreadsFinishedProcessing++;

				if (nrThreadsFinishedProcessing == nrThreads + 1) {
					finishedProcessing = true;
					nrThreadsWaitingForProcessing = 0;
					readyForProcessing = false;
					finishedProcessingCondition.notify_all();
				}
				finishedProcessingCondition.wait(lock, [this] {return finishedProcessing; }); // true indicates that we can restart processing
				 
			}

		}

	}	


	void NodeProcessor::processNodesSingleThread(vector<Node*>* nodes, int mo) {

		Calculator* c = calculators.at(0);

		if (memoryOption == 0) { // no memory, simply use current node as start estimation
			for (int n = 0; n < nodes->size(); n++) {
				c->calculate(nodes->at(n), stopFlag);
			}
		}
		else {
			for (int n = 0; n < nodes->size(); n++) { // use last calculated node as start estimation
				c->calculate2(nodes->at(n), stopFlag);
			}
		}

	}
}


