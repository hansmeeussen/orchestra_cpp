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


		// we do not want to have more threads than nodes
		if (this->nrThreads > nodes->size()) {
			this->nrThreads = nodes->size();
		}

		

		cout << "Nr threads: "<< this->nrThreads << endl;

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

		startProcessing = false;

		// set up and start the threads
		// these will wait until startprocessing is set TRUE 

		for (int n = 0; n < this->nrThreads; n++) {
			// each thread has its own independent calculator, so these can run in parallel
			threads.push_back(new std::thread(&NodeProcessor::runf, this, calculators.at(n)));
		}
	}

	void NodeProcessor::processNodes(vector<Node*>* nodes, int mo) {
		this->memoryOption = mo;
		setSize = nodes->size() / (nrThreads * 10);
		if (setSize < 1) setSize = 1; 
		//setSize = 1; //force a setsize of 1
		// The setSize determines the number of nodes that is processed in one go by each thread. 
		// The optimal setsize is larger for nodes that are related
		processNodes(nodes);
	}

	void NodeProcessor::processNodes(vector<Node*>* nodes) {

		this->nodes = nodes;

		if (nrThreads == 1) {
			processNodesSingleThread(nodes, memoryOption);
			return;
		}


		// Here we just do a single calculation for node 0 and
		// use the results to update the lastSuccessful node of all the calculators in the threads
	    if (memoryOption != 0) {
			// calculate first node with first calculator
			calculators.at(0)->calculate(nodes->at(0), sf);
			for (Calculator* c : calculators) {
				if (c->lastSuccessfulNode2 != nullptr) {
					c->lastSuccessfulNode2->clone(nodes->at(0));
				}
			}
	    }

		// initialize the flag variables and 
		// notify threads that they can start processing
		{
			unique_lock<mutex> lck(mtx);

			nrBusyThreads = 0;
			currentNodeNr = 0;
			lastNodeTaken = false;
			startProcessing = true;
		}
		condition.notify_all(); // notify the waiting worker threads that these can look for startprocessing == TRUE and start

		// Here the processing happens


    	// We wait here until the last node has been taken
		// This is signalled by the getNodes method
		{
			unique_lock<mutex> lck(mtx);
			condition.wait(lck, [this] {return lastNodeTaken; });
		}

		// Now we just have to wait until all busy threads have finished (nrbusy==0)
		{
			unique_lock<mutex> lck(mtx);
			busyCondition.wait(lck, [this] {return isReady(); });
		}

		// Now we are ready. This method returns, but 
		// the worker threads are waiting for the startprocessing barrier

	}


	/*
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
	*/


	//we return nullPtr when all nodes are processed
	// This method gets a single node at the time.
	/*
	Node* NodeProcessor::getNextNode() {
		
		std::lock_guard<mutex> lock(mtx); // use a lock to allow only synchronized access (one thread at the time)
		                                  // this lock is automatically released when it gets out of scope

		// if one of the threads has taken the last node set lastNodeTaken = TRUE
		// the other threads will get a null value for the next node, and stop processing (wait until startProcessing becomes TRUE)

		if (currentNodeNr < nodes->size()) {
			Node* node = (*nodes)[currentNodeNr];
			currentNodeNr++;
			return node;
		} else {
			lastNodeTaken = true;
			startProcessing = false; // we reset the startProcessing variable, so all threads wait until this becomes true
			condition.notify_one(); // notify the waiting main thread
			return  nullptr;
		}
	}
	//*/

	//* New

	/* This method returns a vector with a set of node (pointers) to be calculated 
	   When we are ready a nullptr is returned and the flag variables are set appropriately
	*/

	//*
	vector<Node*>* NodeProcessor::getNextNodes() {

		std::lock_guard<mutex> lock(mtx); 
		// use a lock to allow only synchronized access (one thread at the time)
		// this lock is automatically released when it gets out of scope

        // if one of the treads has processed the last node and has set the processingready flag
        // the other threads that enter this function can stop as well

	
		if (currentNodeNr < nodes->size()) {

			vector<Node*>* ntbc = new vector<Node*>;

			for (int n = 0; n < setSize; n++) {
				if (currentNodeNr < nodes->size()) {
					ntbc->push_back((*nodes)[currentNodeNr]);
					currentNodeNr++;
				}
				else {
					break;
				}
			}
			return ntbc;
		}
		else {
            lastNodeTaken = true;
			startProcessing = false; // we reset the startProcessing variable, so all threads will wait at the startprocessing barrier
			condition.notify_one();  // notify the waiting main thread that is waiting on this condition it will check for the lastNodeTaken flag
			return  nullptr;
		}
	}
	//*/

	void NodeProcessor::pleaseStop() {
		std::lock_guard<mutex> lock(mtx);
		quit = true;
		startProcessing = true;
		// we start processing, but the quit variable will signal to end the nodeprocessor
		// may be we should make sure that it cannot be restarted after pleaseStop()?
		condition.notify_all();
	}

	/*
	// the function that is called by the independent threads
	// each thread uses its own calculator copy
	void NodeProcessor::runf(Calculator* c) {

		while (true) {

			{
				// this is the place where the threads wait until notified to start processing
				unique_lock<mutex> lck(mtx);
				condition.wait(lck, [this] {return startProcessing; }); // so we wait here until goProcessing == TRUE
			}

			// quit if asked to do so (from nodeprocessor destructor)
			if (quit)break;

			incNrBusy(); // increase number of busy threads

			// here the thread will ask for a node and calculate this
			// until all nodes are processed
			while (true) {
				Node* node = getNextNode();
				if (node == nullptr) break; // so we break to the outer loop and wait there for processing
				if (memoryOption == 0) {
			    	c->calculate(node, sf);
			    }else{
				    c->calculate2(node, sf);
			    }
			}

			decNrBusy(); // decrease number of busy threads
		}

	}
	//*/
	
	//*
	// This version processes a set of nodes at the time
	void NodeProcessor::runf(Calculator* c) {

		while (true) {

			{
				// this is the place where the threads wait until notified to start processing
				// so we wait here until waitforprocessing is false
				unique_lock<mutex> lck(mtx);
				condition.wait(lck, [this] {return startProcessing; });
			}

			// quit if asked to do so (from nodeprocessor destructor)
			if (quit) break;

			incNrBusy();

			// here the thread will ask for a (set of) node(s) and calculate this
			// until all nodes are processed
			while (true) {
				vector<Node*>* ntbc = getNextNodes();
				if (ntbc == nullptr) {
					break;
				}


				if (memoryOption == 0) { // no memory, simply use current node as start estimation
	
				    for (int n = 0; n < ntbc->size(); n++) {
						c->calculate(ntbc->at(n), sf);
					}
				}
				else {
					for (int n = 0; n < ntbc->size(); n++) { // use last calculated node as start estimation
						c->calculate2(ntbc->at(n), sf);
					}
				}

				delete(ntbc);
			}

			decNrBusy();

		}

	}	
	//*/

	void NodeProcessor::processNodesSingleThread(vector<Node*>* nodes, int mo) {

		Calculator* c = calculators.at(0);

		if (memoryOption == 0) { // no memory, simply use current node as start estimation
			for (int n = 0; n < nodes->size(); n++) {
				c->calculate(nodes->at(n), sf);
			}
		}
		else {
			for (int n = 0; n < nodes->size(); n++) { // use last calculated node as start estimation
				c->calculate2(nodes->at(n), sf);
			}
		}

	}

	void NodeProcessor::incNrBusy() {
		std::lock_guard<mutex> lock(mtx);
		nrBusyThreads++;
	}

	void NodeProcessor::decNrBusy() {
		{
			std::lock_guard<mutex> lock(mtx);
			nrBusyThreads--;
		}

		if (nrBusyThreads == 0) {
			busyCondition.notify_one();
		}
	}

	bool NodeProcessor::isReady() {
		return nrBusyThreads == 0;
	}
}


