//--------------------------------------------------------------------------------------------------------------------------
// This file is part of the C++ ORCHESTRA chemical solver code
//
// Copyright 2023, J.C.L. (Hans) Meeussen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
//--------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------
// This demonstration program shows how the ORCHESTRA chemical solver can be used from other codes.
// 
//
// Hans Meeussen, March, 2023
//
//
// The following ORCHESTRA object classes are used: 
//
//
// Calculator:
// Represents the chemical solver that is defined by its own text inputfile in terms of substances and reactions etc.
// This input file can be composed / edited with the Java GUI. 
// Calculators operate on Nodes (or cells) which contain all Calculator input and output variables.
// All input - output of calculator variables can only happen via Nodes.
//
//
// Node: 
// A Node (or cell) contains a set of variables that is used as input - output for calculators.
// Variable names that exist in both Node and Calculator are automatically used for IO.
// The Node variables are user-definable, but additionally a Calculator can indicate which variables it wants to store in a Node
// to make recalculation of the Node efficient (e.g. store values of of unknowns).
// A Node contains all necessary state variables to define a system cell and acts as memory between timesteps. 
// It will usually be efficient to create an ORCHESTRA Node for each cell/node in existing code.
// Recalculating a Node that is already in equilibrium is very fast with the stored unknown variables from the previous calculation.
//
//
// NodeType:
// Each Node belongs to a certain NodeType. (Usually there is only one type of nodes in a transport system)
// The NodeType has to be defined before Nodes of this type can be created and defines which variables are stored in each Node of this type. 
//
//
// FileBasket: 
// All ORCHESTRA FileIO happens via a FileBasket.
// Contains filename and working directory, which is internally used by the expander (preprocessor) that may want to open additional
// "included" files in the same directory. 
// 
//
// FileID:
// A File identifyer contains the filename and filebasket.
//  
//
//--------------------------------------------------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "NodeType.h"
#include "Node.h"
#include "Calculator.h"
#include "StopFlag.h"
#include "IO.h"
#include "NodeProcessor.h"
//#include <omp.h> 


using namespace std;
using namespace std::chrono;
using namespace orchestracpp;

int main()
{

	try {

		IO::println("**** ORCHESTRA C++ chemical solver demonstration program, Version February 2021 ");

		//--------------------------------------------------------------------------------------------------------------------------
		// 1: First we create a NodeType object
		// The nodeType determines which variables are stored by nodes (cells) of this type.
		//--------------------------------------------------------------------------------------------------------------------------
		NodeType nodeType;

		//--------------------------------------------------------------------------------------------------------------------------
        // 2: Then we create a FileBasket object which regulates all file IO
        //--------------------------------------------------------------------------------------------------------------------------
		FileBasket* fileBasket = new FileBasket();

		//--------------------------------------------------------------------------------------------------------------------------
		// 3: Now we can use the fileBasket to set the working directory.
		//    Default the current folder is used, but this may depend on the operating system.
		//--------------------------------------------------------------------------------------------------------------------------
		//fileBasket.workingDirectory = "..\\hpx"; // place your workingdirectory here
        //fileBasket.workingDirectory = "C:\\Users\\jmeeussen\\Desktop\\visualC\\testorchestra2\\testorchestra2\\hpx"; // place your workingdirectory here

		//--------------------------------------------------------------------------------------------------------------------------
		// 5:  We create a FileID, using the filebasket,  to open a chemistry inputfile
		//--------------------------------------------------------------------------------------------------------------------------
		FileID fileID(fileBasket, "chemistry1.inp");

		//--------------------------------------------------------------------------------------------------------------------------
        // 6: Now we can construct a calculator object from this input file
        //--------------------------------------------------------------------------------------------------------------------------
		Calculator calculator(&fileID);

		//--------------------------------------------------------------------------------------------------------------------------
        // 6a: Standard the complete definition of the chemical system is read from the chemistry input file, but it is possible to 
		//    add some additional text to the chemistry file (containing arbitrary calculations) which we can provide in an extra string.
		//    This string is inserted at the start of the text of the chemical input file
        //--------------------------------------------------------------------------------------------------------------------------
		//std::string extratext("@class: hpxtxt(){@Globalvar: hpxpipi 3.14}");
		//Calculator calculator(&fileID, extratext); // providing addtional text

		//Calculator calculator2 = *calculator1.clone(); // this is how we can clone a calculator


		//--------------------------------------------------------------------------------------------------------------------------
		// 7: We now ask the calculator for all the variables that it wants to store per node.
		//    This will include all variables that are defined as global variables in the calculator (plus their default values),
		//    and all variables that are used as unknown / equation in the solver.
		//    The latter is important for efficiency, as old unknown values can in this way be used as start estimations for unknowns
		//    in a subsequent calculation.
		//--------------------------------------------------------------------------------------------------------------------------
		nodeType.useGlobalVariablesFromCalculator(&calculator);

		// Just for a test we print all these variables and their default values
		IO::println("The calculator " + calculator.name->name+" wants the following global variables to be stored in each cell :\n");
		for (int n = 0; n < nodeType.getNrVars(); n++) {
			cout << n << " : " << nodeType.getName(n) << endl;
		}

		//--------------------------------------------------------------------------------------------------------------------------
		// 8: We also need a flag to stop the calculator(s) externally if necessary, which is needed in case of multithreaded systems 
		//--------------------------------------------------------------------------------------------------------------------------
		StopFlag* stopFlag = new StopFlag();
		//stopFlag->pleaseStop("demo program line 126");// this is how we can stop all running calculators

		//--------------------------------------------------------------------------------------------------------------------------
        // 9: Now we can add some extra variables that we would like to use for input/output
		//    addVariable(<name>, <default value>, <is static>, <origin>)
		//    a static variable has a single value that is shared by all cells/nodes of the same nodeType
		//    the origin string is just a name to identify where the variable was created.
		//    ANY variable in the chemical sysem is available for IO in this way!
		//--------------------------------------------------------------------------------------------------------------------------

		nodeType.addVariable("tot_nr_iter", 0, false, "test");
		nodeType.addVariable("pH",          7, false, "test");
		nodeType.addVariable("O2.logact", -1,  false, "test");		


		//to access these variables efficiently we create indices to them
		int indx_pH     = nodeType.index("pH");
		int indx_nriter = nodeType.index("tot_nr_iter");
		int indx_O2     = nodeType.index("O2.logact");
		int indx_id     = nodeType.index("Node_ID");
		int indx_Ca     = nodeType.index("Ca+2.tot");
		int indx_pe     = nodeType.index("pe");

		//--------------------------------------------------------------------------------------------------------------------------
		// 10: Now we are ready to create a new node
		Node node1(&nodeType);
		//--------------------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------------------
		// 11: Write output for all node variables before calculation
		//--------------------------------------------------------------------------------------------------------------------------
		cout.precision(14);
		for (int n = 0; n < nodeType.getNrVars(); n++) {
			cout<<n << " : " << nodeType.getName(n)<<   " : "<<   node1.getvalue(n)  <<endl;
		}

		//--------------------------------------------------------------------------------------------------------------------------
		// 12: Set the pH of the input 
		//--------------------------------------------------------------------------------------------------------------------------
		node1.setValue(indx_pH, 7.0);
		node1.setValue(indx_pe, 7.0);

		//--------------------------------------------------------------------------------------------------------------------------
		// 13: and perform a calculation on this node	
		//--------------------------------------------------------------------------------------------------------------------------
		bool success = false;
		try {
			success = calculator.calculate(&node1, stopFlag);
		}
		catch (OrchestraException  e)
		{   // If the calculation fails, an exception is thrown
			IO::showMessage("Something went wrong trying to calculate a node: " + string(e.what()));
		}
		//--------------------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------------------
		// 14: write output for all node variables after calculation
		//--------------------------------------------------------------------------------------------------------------------------
		if (success) {
			cout << "The first ORCHESTRA calculation succeeded!" << endl;
			for (int n = 0; n < nodeType.getNrVars(); n++) {
				cout << n << " : " << nodeType.getName(n) << " : " << node1.getvalue(n) << endl;
			}
		}
		else 
		{
			cout << "Sorry, for some reason the first ORCHESTRA calculation was not successful!" << endl;
		}




		//--------------------------------------------------------------------------------------------------------------------------
		// Below are just some examples of different ways to perform multiple calculations on single or series of nodes 
		//
		//--------------------------------------------------------------------------------------------------------------------------

		if (false) { // just to switch this off temporarily
//	    //	 just perform a series of calculations (1000) with a range of pH's
//		try {
//	
//			for (double pH = 7.0; pH <= 10; pH = pH + (3.0 / 1000.0)) {
//			    node1.setValue(indx_pH, pH);
//			    success = calculator.calculate(&node1, stopFlag);
//				cout<<".";
//		    }
//
//		}
//		catch (OrchestraException  e)
//		{   // If the calculation fails, an exception is thwn
//			IO::showMessage("Something went wrong trying to calculate a node: " + string(e.what()));
//		}
//		//--
			//--------------------------------------------------------------------------------------------------------------------------
			// 15: Now we show how to calculate a larger sets of nodes in different ways
			//     comment out one of the following methods and compare
			//--------------------------------------------------------------------------------------------------------------------------
			int nrcalc = 0;
			int totnriter = 0;

			// we create 3 sets of nodes copies
			std::vector<Node*>* theNodes = new std::vector<Node*>;
			std::vector<Node*>* theNodes2 = new std::vector<Node*>;
			std::vector<Node*>* theNodes3 = new std::vector<Node*>;

			// construct 3 x 40.000 new nodes and give each node a different pH
			double pH = 8.0;
			//for (double pH = 4.0; pH < 8.0; pH = pH + 1.0e-4) {
			for (double Ca = 1e-4; Ca < 1; Ca = Ca + 1e-4) { // 10.000 nodes

				Node* tmpNode = node1.clone();  // we copy the first node

				tmpNode->setValue(indx_Ca, Ca); // 			
				tmpNode->setValue(indx_pH, pH); // and set the pH at a new value

				// add node copies to all 3 node sets, so we get 3 identical sets of nodes
				theNodes->push_back(tmpNode);
				theNodes2->push_back(tmpNode->clone());
				//theNodes3->push_back(tmpNode->clone());
			}

			// we also make a smaller set of 100 nodes, to calculate this 100 times and compare with 10000 nodes in one set
			for (double Ca = 1e-4; Ca < 1; Ca = Ca + 1e-2) { //100 nodes

				Node* tmpNode = node1.clone();  // we copy the first node

				tmpNode->setValue(indx_Ca, Ca); // 			
				tmpNode->setValue(indx_pH, pH); // and set the pH at a new value
				theNodes3->push_back(tmpNode->clone());
			}

			//--------------------------------------------------------------------------------------------------------------------------
			// A: simply calculate all nodes sequentially as independent nodes
			//--------------------------------------------------------------------------------------------------------------------------
			auto t0 = high_resolution_clock::now();

			for (int n = 0; n < theNodes->size(); n++) {
				calculator.calculate(theNodes->at(n), stopFlag);
				totnriter += (int)theNodes->at(n)->getvalue(indx_nriter); // we add the number of iterations and add this to the total number of iterations
			}

			auto t1 = high_resolution_clock::now();
			auto duration = duration_cast<milliseconds>(t1 - t0).count();
			cout << "A: Sequential number of calculations: " << theNodes->size() << " Total nr iterations: " << totnriter << "  time: " << duration << "msec" << endl;

			//--------------------------------------------------------------------------------------------------------------------------
			// B: calculate all nodes sequentially, but making use of results of previous node as start estimation for the next one.
			// This is often useful for initializing large systems with many related nodes
			//--------------------------------------------------------------------------------------------------------------------------
			t0 = high_resolution_clock::now();
			totnriter = 0;

			for (int n = 0; n < theNodes2->size(); n++) {
				if (n > 0) {
					calculator.copyUnknowns(theNodes2->at(n - 1), theNodes2->at(n));
				}
				calculator.calculate(theNodes2->at(n), stopFlag);
				totnriter += (int)theNodes2->at(n)->getvalue(indx_nriter);
			}

			t1 = high_resolution_clock::now();
			duration = duration_cast<milliseconds>(t1 - t0).count();
			cout << "B: Sequential number of calculations: " << theNodes->size() << " Total nr iterations: " << totnriter << "  time: " << duration << "msec" << endl;

			//--------------------------------------------------------------------------------------------------------------------------
			// C: Repeat calculations, so nodes are already in equilibrium
			//--------------------------------------------------------------------------------------------------------------------------
			t0 = high_resolution_clock::now();
			totnriter = 0;

			for (int n = 0; n < theNodes2->size(); n++) {
				calculator.calculate(theNodes2->at(n), stopFlag);
				totnriter += (int)theNodes2->at(n)->getvalue(indx_nriter);
			}

			t1 = high_resolution_clock::now();
			duration = duration_cast<milliseconds>(t1 - t0).count();
			cout << "C: Sequential number of calculations: " << theNodes->size() << " Total nr iterations: " << totnriter << "  time: " << duration << "msec" << endl;


			//--------------------------------------------------------------------------------------------------------------------------

			// Calculate independent nodes in parallel threads using the parallel node processor
			// The NodeProcessor will make and use internal independent copies of the calculator for each thread
			// Initialize the NodeProcessor, this needs to happen once. (and is relatively slow)
			// After that, the processNodes() method can be called repeatedly (and is relatively fast)
			NodeProcessor np(&calculator, 8, stopFlag, theNodes); // 8 threads

			t0 = high_resolution_clock::now();

			// process the nodes in parallel, can be used repeatedly
			for (int m = 0; m < 100; m++) {
				np.processNodes(theNodes3);
			}

			t1 = high_resolution_clock::now();

			duration = duration_cast<milliseconds>(t1 - t0).count();
			cout << "D: Parallel number of calculations: " << theNodes3->size() << "  time: " << duration << "msec" << endl;



			// process the nodes in parallel, can be used repeatedly so do it again
			// this will be quite fast, as nodes are already equilibrated once
			t0 = high_resolution_clock::now();
			for (int m = 0; m < 100; m++) {
				np.processNodes(theNodes3);
			}

			t1 = high_resolution_clock::now();
			duration = duration_cast<milliseconds>(t1 - t0).count();
			cout << "E: repeat parallel number of calculations: " << theNodes3->size() << "  time: " << duration << "msec" << endl;



			t0 = high_resolution_clock::now();
			for (int m = 0; m < 100; m++) {
				np.processNodes(theNodes3);
			}

			t1 = high_resolution_clock::now();
			duration = duration_cast<milliseconds>(t1 - t0).count();
			cout << "F: repeat parallel number of calculations: " << theNodes3->size() << "  time: " << duration << "msec" << endl;



			t0 = high_resolution_clock::now();
			for (int m = 0; m < 100; m++) {
				np.processNodes(theNodes3);
			}

			t1 = high_resolution_clock::now();
			duration = duration_cast<milliseconds>(t1 - t0).count();
			cout << "G: repeat parallel number of calculations: " << theNodes3->size() << "  time: " << duration << "msec" << endl;


			t0 = high_resolution_clock::now();
			//for (int m = 0; m < 100; m++) {
			np.processNodes(theNodes2);
			//

			t1 = high_resolution_clock::now();
			duration = duration_cast<milliseconds>(t1 - t0).count();
			cout << "H: repeat parallel number of calculations: " << theNodes3->size() << "  time: " << duration << "msec" << endl;
		}
	}
	catch (const IOException &e)
	{
		cout << e.what() << endl;
	}

 	catch (const OrchestraException &e) {
		cout << e.what() << endl;
	}
	return 0;

} 

