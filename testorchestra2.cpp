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
// See the License for the specific language governing permissions and
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
// In contrast with PHREEQC the input file contains all chemical info required, so no additional access to databases is necessary.
//
// The input file can be composed / edited with the Java GUI. (which does use thermodynamic (PHREEQC format) databases)
// 
// Calculators operate on Nodes (or cells) which contain the input and output variables for the solver
// All input - output of calculator variables needs to happen via the Nodes.
//
//
// Node: 
// A Node (or cell) contains a set of variables that is used as input - output for calculators.
// Variable names that exist in both Node and Calculator are automatically used for communication between solver and node.
// The Node variables are user-definable, but additionally a Calculator can indicate which variables it wants to store in a Node
// to make recalculation of the Node efficient (e.g. store values of unknowns).
// A Node contains all necessary state variables to define a system cell and acts as memory between timesteps. 
// It will usually be efficient to create an ORCHESTRA Node for each cell/node in transport code.
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


using namespace std;
using namespace std::chrono;
using namespace orchestracpp;


// This function performs a complete test and writes results to report file
void test(string name, vector<Node*>* nodes, NodeProcessor* np, FileWriter* report, FileBasket fileBasket, ParameterList* outputVariableNames, vector<int>* outputIndx, int memoryOption, bool writeOutput) {
	int nrThreads = std::thread::hardware_concurrency();
	int iterindex = nodes->at(0)->nodeType->index("tot_nr_iter");

	auto t0 = high_resolution_clock::now();
	// perform the calculations
	np->processNodes(nodes, memoryOption);
	auto t1 = high_resolution_clock::now();
	// determine the calculation time
	auto duration = duration_cast<milliseconds>(t1 - t0).count();
	if (duration < 1)duration = 1;
	int tot_nr_iter = 0;

	// determine total number of required iterations sum for all nodes
	for (int n = 0; n < nodes->size(); n++) { tot_nr_iter += nodes->at(n)->getvalue(iterindex); }


	// write result to report file and screen
	string result = (name + ": \t" + to_string(nodes->size()) + " nodes,   calculation time: " + to_string(duration) + " msec,  Calculations/sec:  " + to_string(nodes->size() * 1000 / duration) + " Total number of iterations: " + to_string(tot_nr_iter) + "\n");
	cout << result;
	report->write(result);

	// write calculated data to output file for checking
	if (writeOutput) {

		FileWriter* fw = fileBasket.getFileWriter(&fileBasket, "output_" + name + ".txt");

		// write header 
		for (int n = 0; n < outputVariableNames->size(); n++) {
			fw->write(outputVariableNames->get(n));
			fw->write("\t");
		}
		fw->write("\n");

		// write data
		for (int n = 0; n < nodes->size(); n++) {
			for (int i = 0; i < outputIndx->size(); i++) {
				//				fw->write(StringHelper::toString(nodes->at(n)->getvalue(outputIndx->at(i))));
				fw->write(StringHelper::doubleToString(nodes->at(n)->getvalue(outputIndx->at(i)), 12));
				//fw->write(nodes->at(n)->getvalue(outputIndx->at(i)));
				fw->write("\t");
			}
			fw->write("\n");
		}
		fw->write("\n");

		fw->close();

		delete fw;
	}
}



int main()
{

	try {

		IO::println("**** ORCHESTRA C++ chemical solver demonstration program, Version Januari 2024 ");

		//--------------------------------------------------------------------------------------------------------------------------
		// 1: First we create a NodeType object
		// The nodeType determines which variables are stored by nodes (cells) of this type.
		//--------------------------------------------------------------------------------------------------------------------------
		NodeType nodeType;

		//--------------------------------------------------------------------------------------------------------------------------
		// 2: Then we create a FileBasket object which regulates all file IO
		//--------------------------------------------------------------------------------------------------------------------------
		FileBasket fileBasket;

		//--------------------------------------------------------------------------------------------------------------------------
		// 3: Now we can use the fileBasket to set the working directory.
		//    Default the current folder is used, but this may depend on the operating system.
		//--------------------------------------------------------------------------------------------------------------------------
		//fileBasket.workingDirectory = "..\\hpx"; // place your workingdirectory here
		//fileBasket.workingDirectory = "C:\\Users\\jmeeussen\\Desktop\\visualC\\testorchestra2\\testorchestra2\\hpx"; // place your workingdirectory here

		//--------------------------------------------------------------------------------------------------------------------------
		// 4:  We create a FileID, using the filebasket,  to open a chemistry inputfile for the solver
		//--------------------------------------------------------------------------------------------------------------------------
		FileID fileID(&fileBasket, "chemistry1.inp");
		//FileID fileID2(&fileBasket, "convertInput.inp"); 

		//--------------------------------------------------------------------------------------------------------------------------
		// 5: Now we can construct a calculator (solver) object from this input file
		//    We can have multiple different calculators if necessary (e.g. to calculate different boundary conditions, 
		//    or to convert input value)
		//--------------------------------------------------------------------------------------------------------------------------
		Calculator calculator(&fileID);
		//Calculator calculator(new FileID(&fileBasket, "chemistry1.inp"));
		//Calculator convertInput(&fileID2);// a calculator to perform initial calculations

		//--------------------------------------------------------------------------------------------------------------------------
		// 5a: Standard the complete definition of the chemical system is read from the chemistry input file, but it is possible to 
		//    add some additional text to the chemistry file (e.g. containing extra calculations) which we can provide in an extra string.
		//    This string is inserted at the start of the text of the chemical input file. 
		//    Used by HPX
		//--------------------------------------------------------------------------------------------------------------------------
		//std::string extratext("@class: hpxtxt(){@Globalvar: hpxpipi 3.14}");
		//Calculator calculator(&fileID, extratext); // providing addtional text

		//Calculator calculator2 = *calculator1.clone(); // this is how we can clone a calculator




		//--------------------------------------------------------------------------------------------------------------------------
		// 6: read all input data points from the file input.dat
		//--------------------------------------------------------------------------------------------------------------------------

		OrchestraReader* inputReader = OrchestraReader::getOrchestraFileReader(&fileBasket, "input.dat");


		string line;
		// skip initial comment lines
		do {
			line = StringHelper::trim(inputReader->readLine());
		} while (StringHelper::startsWith(line, "#") || (line.size() < 1));


		// read the line with input variable names in the column headers
		ParameterList inputVariableNames(line);

		int nrColumns = inputVariableNames.size();

		//  now read the data lines
		std::vector<ParameterList*> dataLines;

		do {
			line = StringHelper::trim(inputReader->readLine());
			// we could check whether the number of data columns in this line agrees with the number of variable names in the column header
			ParameterList* inputDataLine = new ParameterList(line);
			if (inputDataLine->size() > 0) {
				dataLines.push_back(inputDataLine);
			}
		} while (!inputReader->ready);

		cout << "We have " << dataLines.size() << " datapoints in input file!" << "\n";
		cout << "We have " << inputVariableNames.size() << " variables in input file!" << "\n";

		//--------------------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------------------
		// 7: Read the required output variable names from the column headers in output.dat
		//    Normally we would also write the output to this file, but in this case we create different output files but 
		//    use the variables defined here. 
		//--------------------------------------------------------------------------------------------------------------------------
		OrchestraReader* outputReader = OrchestraReader::getOrchestraFileReader(&fileBasket, "output.dat");


		// skip the comment lines
		do {
			line = StringHelper::trim(outputReader->readLine());
		} while (StringHelper::startsWith(line, "#") || (line.size() < 1));

		// read the variable names in the column headers
		ParameterList outputVariableNames(line);

		cout << "We have " << outputVariableNames.size() << " variables in output file!" << "\n";
		//--------------------------------------------------------------------------------------------------------------------------


		//--------------------------------------------------------------------------------------------------------------------------
		// 8: We now ask the calculator for all the variables that it wants to store per node.
		//    This will include all variables that are defined as global variables in the calculator (plus their default values),
		//    and all variables that are used as unknown / equation in the solver.
		//    The latter is important for efficiency, as old unknown values can in this way be used as start estimations for unknowns
		//    in a subsequent calculation.
		//    We do this for all calculators in the system 
		// 
		//    In this way we have defined all variables that are stored in each node.
		//--------------------------------------------------------------------------------------------------------------------------
		//nodeType.useGlobalVariablesFromCalculator(&calculator);
		//nodeType.useGlobalVariablesFromCalculator(&convertInput);


		//nodeType.readGlobalVariablesFromOutputFile(fileBasket, "output.dat");
		cout << "We have are reading the nodeType from output.dat!" << "\n";

		nodeType.readGlobalVariablesFromOutputFile(&fileBasket, "output.dat");

		//&calculator.addGlobalVariables(nodeType.outputvariables);
		calculator.addGlobalVariables(&nodeType.outputVariables);

		nodeType.useGlobalVariablesFromCalculator(&calculator);

		//nodeType.readGlobalVariablesFromInputFile(fileBasket, "input.dat");
		//nodeType.readGlobalVariablesFromFile(&fileBasket, "input.dat");


		//--------------------------------------------------------------------------------------------------------------------------
		// 9: Now we can add the input and output variables to the nodeType 
		//    The "false" parameter indicates that each node has an individual value for this variable 
		//    This in contrast with "true" which indicates static variables of which there is a single copy for all nodes (e.g. time or timestep etc.)
		//    It is also possible to indicate where the variables definition originates from, in this case the input and output file.
		//--------------------------------------------------------------------------------------------------------------------------
		for (int n = 0; n < inputVariableNames.size(); n++) {
			nodeType.addVariable(inputVariableNames.get(n), 0, false, "input.dat");
		}

		// do the same for output variables
		for (int n = 0; n < outputVariableNames.size(); n++) {
			nodeType.addVariable(outputVariableNames.get(n), 0, false, "output.dat");
		}
		//--------------------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------------------
		//    We have now defined all variables that are stored in each node or cell.
		// 
		//--------------------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------------------
		// 10: Now we can create just as many nodes as there are input datapoints
		//--------------------------------------------------------------------------------------------------------------------------
		cout << "We are creating " << dataLines.size() << " nodes!" << "\n";

		vector<Node*> nodes;
		for (int n = 0; n < dataLines.size(); n++) {
			nodes.push_back(new Node(&nodeType));
		}

		//--------------------------------------------------------------------------------------------------------------------------
		// 11: For fast access to node variables we create integer indices to them
		//--------------------------------------------------------------------------------------------------------------------------
		// we do this for input..
		vector<int> inputIndx;
		for (int n = 0; n < inputVariableNames.size(); n++) {
			inputIndx.push_back(nodeType.index(inputVariableNames.get(n)));
		}

		// as well as output variables.
		vector<int> outputIndx;
		for (int n = 0; n < outputVariableNames.size(); n++) {
			outputIndx.push_back(nodeType.index(outputVariableNames.get(n)));
		}
		//--------------------------------------------------------------------------------------------------------------------------



		//--------------------------------------------------------------------------------------------------------------------------
		// 12: Now we can use the input indices to set the node variables with data read from the input file
		//--------------------------------------------------------------------------------------------------------------------------
		for (int n = 0; n < dataLines.size(); n++) {
			for (int i = 0; i < dataLines[n]->size(); i++) {
				nodes[n]->setValue(inputIndx[i], dataLines[n]->getDouble(i));
			}
		}

		//--------------------------------------------------------------------------------------------------------------------------
		// 13: Now we create 2 so-called nodeProcessors, the first one to perform single thread calculations,
		//     the second one to perform parallel multi threaded calculations
		//--------------------------------------------------------------------------------------------------------------------------

		// we also need a stopflag, which can be used to stop long running calculations running in the background in e.g. interactive calculations (not used here)
		StopFlag* stopFlag = new StopFlag();
		//stopFlag->pleaseStop("demo program line 266");// this is how we can stop all running calculators

		NodeProcessor single(&calculator, 1, stopFlag, &nodes); // single calculator 
		cout << "We have created a NodeProcessor!" << "\n";
		NodeProcessor multi(&calculator, -1, stopFlag, &nodes); // negative number of calculators : automatically determine number of threads for the chemistry calculations		

		// we need 4 copies of the nodes to perform the benchmark on
		vector<Node*> nodes_random_single;
		vector<Node*> nodes_random_multi;
//	 	vector<Node*> nodes_sorted_single;
//		vector<Node*> nodes_sorted_multi;

		// we clone the input nodes
		for (int i = 0; i < nodes.size(); i++) {
			nodes_random_single.push_back(nodes.at(i)->clone());
			nodes_random_multi.push_back(nodes.at(i)->clone());
//			nodes_sorted_single.push_back(nodes.at(i)->clone());
//			nodes_sorted_multi.push_back(nodes.at(i)->clone());
		}

		// we sort 2 of the 4 sets on one of the input variables
		/**
		std::string sortName;

		if (nodeType.index("CaO") > 1) {
			sortName = "CaO";
		}
		else if (nodeType.index("acidbase") > 1) {
			sortName = "acidbase";
		}
		else {
			sortName = nodeType.getName(0);
		}
		single.sortNodes(&nodes_sorted_single, sortName);
		single.sortNodes(&nodes_sorted_multi, sortName);
		*/

		FileWriter* report = fileBasket.getFileWriter(&fileBasket, "report.txt");

		// this will report the nr of logical processors, which may be double the number of physical processors in case of hyperthreading 
		int nrThreads = std::thread::hardware_concurrency();

		report->write("#\n# DONUT Machine Learning Benchmark:  calculation times with a traditional chemical solver.\n");
		report->write("# (ORCHESTRA C++ version as developed within DONUT project)\n#\n");
		report->write("# Calculation times of a chemical solver depend very strongly on the number of iterations required to solve a system,\n");
		report->write("# which in turn is very sensitive to the accuracy of the start estimations.\n");
		report->write("# This benchmark demonstrates this by performing a series calculations of random (unrelated) and related (sorted) chemical systems.\n");
		report->write("# In both cases, the results of a the previous calculation are used as start estimation for a new one.\n#\n");
		report->write("# For ordered sets the results of a previous calculation are a better start estimation for a new calculation than for random sets,\n# resulting in less required iterations and faster calculation times for ordered sets.\n#\n");
		report->write("# For transport systems usually the results of the previous time step (for each cell or node) are used as start estimations.\n");
		report->write("# These estimations are typically very good, as changes between time steps are small, (or even no changes in large part of the system). \n\n");
		report->write("# For that reason, the performance of a chemical solver in transport systems is likely to be closer to the results for warm start conditions, than those for random input. \n");
		report->write("# \n");
		report->write("# This benchmark furthermore demonstrates the efficiency of parallel calculations on systems with multiple processors / calculation cores.\n");
		report->write("# Note that especially on laptop computers, processor speeds are often reduced when all cores are used to reduce power consumption and heat production.\n");
		report->write("# This results in less than linear scaling of calculation speed with number of processors/threads.\n#\n");
		report->write("# Hans Meeussen, 24 Januari 2024.\n#\n");


		int iterindex = nodeType.index("tot_nr_iter");


		//--------------------------------------------------------------------------------------------------------------------------
		// 15: Now we can perform the different runs and write the results to screen and report.txt file
		//--------------------------------------------------------------------------------------------------------------------------

		// This was just to test a single calculationL
		// calculator.calculate(nodes[0], stopFlag);
		//test::t(string name, vector<Node*>*nodes, NodeProcessor * np, FileWriter * report, FileBasket fileBasket, ParameterList * outputVariableNames, vector<int>*outputIndx, memoryOption, writeResults) {
		test("single_thread_random", &nodes_random_single, &single, report, fileBasket, &outputVariableNames, &outputIndx, 1, true);
		//test("single_thread_sorted", &nodes_sorted_single, &single, report, fileBasket, &outputVariableNames, &outputIndx, 1, true);
		test(to_string(nrThreads) + "_threads_random", &nodes_random_multi, &multi, report, fileBasket, &outputVariableNames, &outputIndx, 1, true);
		//test(to_string(nrThreads) + "_threads_sorted", &nodes_sorted_multi, &multi, report, fileBasket, &outputVariableNames, &outputIndx, 1, true);

		report->write("# \n");
		report->write("# Now we redo the calculations to demonstrate the effect of a warm start with very good start estimations\n");
		report->write("# The performance of a solver for transport systems is typically closer to the results for warm start conditions, than those for random input.\n");
		report->write("# We expect no significant difference anymore between ordered / non ordered as for each cell the conditions of the previous calculations for this cell are used.\n");
		report->write("# Because the calculations will now(most likely) be much faster than the previous ones, the overhead of multi threading is relatively more important.\n");
		report->write("# Good scaling of calculation speed with number of threads indicates low overhead of multithreading.\n");
		report->write("# \n");

		// we do not write output and memoryoption == 0, which implies that we use the results of the previous calcultion for this node as start estimation
		test("single_thread_random", &nodes_random_single, &single, report, fileBasket, &outputVariableNames, &outputIndx, 0, false);
		//test("single_thread_sorted", &nodes_sorted_single, &single, report, fileBasket, &outputVariableNames, &outputIndx, 0, false);
		test(to_string(nrThreads) + "_threads_random", &nodes_random_multi, &multi, report, fileBasket, &outputVariableNames, &outputIndx, 0, false);
		//test(to_string(nrThreads) + "_threads_sorted", &nodes_sorted_multi, &multi, report, fileBasket, &outputVariableNames, &outputIndx, 0, false);

		report->write("# \n");
		report->write("# Now we do this again to check reproducibility...\n");
		report->write("# \n");

		for (int n = 0; n < 10; n++) {
			test("single_thread_random", &nodes_random_single, &single, report, fileBasket, &outputVariableNames, &outputIndx, 0, false);
			//test("single_thread_sorted", &nodes_sorted_single, &single, report, fileBasket, &outputVariableNames, &outputIndx, 0, false);
			test(to_string(nrThreads) + "_threads_random", &nodes_random_multi, &multi, report, fileBasket, &outputVariableNames, &outputIndx, 0, false);
			//test(to_string(nrThreads) + "_threads_sorted", &nodes_sorted_multi, &multi, report, fileBasket, &outputVariableNames, &outputIndx, 0, false);
		}


		report->write("# \n");
		report->write("# Now we repeat the calculations to fully use the processor...\n");
		report->write("# \n");
		// now we do some hard work
		for (int n = 0; n < 30; n++) {
			nodes_random_multi.clear();
			for (int i = 0; i < nodes.size(); i++) {
				//	nodes_random_single.push_back(nodes.at(i)->clone());
				nodes_random_multi.push_back(nodes.at(i)->clone());
			}
			test(to_string(nrThreads) + "_threads_random", &nodes_random_multi, &multi, report, fileBasket, &outputVariableNames, &outputIndx, 1, false);
		}

		// we can wait 5 minutes and start again?

		report->close();

	}
	catch (const IOException& e)
	{
		cout << e.what() << endl;
	}

	catch (const OrchestraException& e) {
		cout << e.what() << endl;
	}
	return 0;

}


