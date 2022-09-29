 #include "Calculator.h"
#include "stringhelper.h"
#include "Expander.h"

#include <chrono>

// this is new
using namespace std::chrono;

namespace orchestracpp

{
	Calculator::Calculator()
	{
		variables   = new VarGroup();
		expressions = new ExpressionGraph(variables);
		uneqs       = new UnEqGroup(variables);
	}

	Calculator::Calculator(std::string filename) : Calculator( new FileID(nullptr, filename)){
	}

	Calculator::Calculator(FileID* name) : Calculator(name, "") {
	}

	Calculator::Calculator(FileID* name, std::string extraText) : Calculator()
	{
		this->name = name;
		optimized = false;
		IO::println("Reading and expanding calculator new style" + name->name);
		auto t0 = high_resolution_clock::now();
		try {
			std::string text;

			text.append("@class: chemistry_file(){" + name->name + "}\n");
			text.append(extraText + "\n");
			text.append("@scan: @chemistry_file()\n");

			text.append("@ini_phases()\n");
			text.append("@scan: @chemistry_file()\n");
			text.append("@use_phases2()\n");

			text.append("@Var: nr_iter  0\n");
			text.append("@Var: tot_nr_iter  0\n");
			text.append("@Var: failed   0\n");
			text.append("@GlobalVar: minTol   1e-4\n");
			text.append("@Var: Node_ID   0\n");
			 
			text.append("@include: @chemistry_file()\n");

			StringWriter tmpwriter;
			Expander expander;
			OrchestraReader tmpReader(text);
			expander.expand(&tmpReader, &tmpwriter, name->basket);
			expandedText = tmpwriter.toString();

			auto t1 = high_resolution_clock::now();
			auto duration = duration_cast<milliseconds>(t1 - t0).count();
			IO::print(StringHelper::toString((double)duration / 1000.0));
			IO::println(" sec.");

			// write expanded text for testing
			FileWriter* writer = FileBasket::getFileWriter(nullptr, "expanded_cpp.txt");
			writer->write(expandedText);
			writer->close();
			delete writer;
			


			readSystemFromExpandedInput(expandedText);
			IO::print(expandedText);
		}
		catch (const OrchestraException& e)
		{
			throw ReadException("Could not open calculator input file file! \n" + std::string(e.what()));
		}
	}
	
	Calculator::Calculator(const Calculator *oldCalculator): Calculator() //throw(ReadException)
	{
		try
		{
			this->name  = oldCalculator->name;
			optimized   = false;
			expandedText = oldCalculator->expandedText;
			readSystemFromExpandedInput(expandedText);
		}
		catch (const IOException &e)
		{
			std::string w = e.what();
			throw ReadException(w + "\nCould not open " + name->name + " to read calculator!");
		}
	}

	Calculator *Calculator::clone()
	{
		try
		{
			return new Calculator(this);
		}
		catch (const ReadException &re)
		{
			std::string w = re.what();
			throw ReadException(w + "\nSomething went wrong cloning calculator  " + name->name + " !");
		}
		return nullptr;
	}

	void Calculator::readSystemFromExpandedInput(const std::string &text)// throw(IOException, ReadException)
	{
		auto t0 = high_resolution_clock::now();

		IO::print("\tReading variables .... ");
		OrchestraReader tmpReader(text);
		readVariablesAndExpressions(&tmpReader);

		auto t1 = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(t1 - t0).count();
		IO::print(StringHelper::toString((double)duration / 1000.0));	IO::println(" s");
	}

	void Calculator::readVariablesAndExpressions(OrchestraReader *infile)// throw(ReadException, IOException)
	{
	   // ArrayList<String> uneqlines = new ArrayList();
		std::vector<std::string> uneq2lines;
		std::vector<std::string> uneq3lines;

		infile->stripComment = true;
		try
		{
			while (!infile->ready)
			{
				std::string word = infile->readWord();

				if (!StringHelper::endsWith(word, ":"))
				{
					// this is not a keyword, so skip the rest of this loop
					continue;
				}

				word = StringHelper::toLower(word);

				if ((word == "@var:") || (word == "var:")) {
					variables->readOne(infile);
				}
				else if (word == "@globalvar:") {
					variables->addToGlobalVariables(variables->readOne(infile));
				}
				else if (word == "@synonym:") {
					variables->createSynonym(infile);
				}
				else if ((word == "@stage:") || (word == "@calc:")) {
					expressions->readExpression(infile);
				}
				else if ((word == "@uneq:") || (word == "uneq:")) {
					IO::showMessage("@Uneq: type has become obsolete, please replace with Uneq2:");
				}
				else if ((word == "@solve:")||(word == "@uneq2:")){
					std::string tmpLine = infile->readLine();
					if (tmpLine.find("si:") != std::string::npos)
					{
						// this is a mineral
						uneq3lines.push_back(tmpLine);
					}
					else
					{
						uneq2lines.push_back(tmpLine);
					}
				}
				else if (word == "@uneq3:") { // we should use a solve command here as well. How do we know it is a mineral? Because it contains an "si" parameter
					uneq3lines.push_back(infile->readLine());
				}
				else if ((word == "@out:") || (word == "out:")) {
					variables->addToGlobalVariables(infile->readWord());
				}
				else if ((word == "@global:") || (word == "global:")) {
					variables->addToGlobalVariables(infile->readWord());
				}
				else if (word == "@maxtry:") {
					maxtry = infile->readInt();
				}
				else if (word == "@maxiter:") {
					uneqs->maxIter = infile->readInt();
				}
				else if (word == "@maxmineraliter:") {
					uneqs->maxMineralIterations = infile->readInt();
				}
				else if (word == "@stop:") {
					stopIfFailed = true;
					exitIfFailed = true;
				}
				else if (word == "@exit:") {
					std::cout << this->name << ": Exit if failedtrue" << std::endl;
					exitIfFailed = true;
				}
				else if (word == "@switchoniia:") {
					switchOnIIA = true;
				}
				else if (word == "@trysteps:") {
					nrIntermediateNodes = infile->readInt();
				}
						
			}

			nrIterVar = variables->get("nr_iter");
			totalNrIterVar = variables->get("tot_nr_iter");

		}

		catch (OrchestraException f)
		{
			std::string w = f.what();
			throw ReadException(w + ", in the file " + name->name);
		}

		for (auto s : uneq2lines)
		{
			uneqs->read_one2(s);
		}

		for (auto s : uneq3lines)
		{
			uneqs->read_one3(s);
		}

	}

	bool Calculator::calculate(Node *const node, StopFlag *flag) //throw(ReadException, ParserException, ExitException)
	{
		calculatorStopFlag = flag;

		// Here we store the original node
		if (orgNode == nullptr)
		{
		   orgNode = new Node(node->nodeType);
		}
		orgNode->clone(node);

		bool calculationSuccessful;

		totNrIterations = 0;

		// create an IO object when called for the first time
		if (iob1 == nullptr)
		{ // first calculation
			iob1 = new NodeIOObject("", variables, node->nodeType);
			failedIndex = node->nodeType->index("failed");
			nodeIDIndex = node->nodeType->index("Node_ID");
		}

		//**
		 //* Create a new last successful node if required We could also do this
		 //* after the first calculation
		 //*/
		if (localLastSuccessfulNode == nullptr)
		{
			// The first calculation with this calculator
			localLastSuccessfulNode = new Node(node->nodeType); // node with default values
		}


		trylevel = 0;
		trying = false; //29-9-2014

	// Here we start the actual calculation
		if (uneqs->getIIApresent())
		{

			if (switchOnIIA)
			{ // start with all uneqs switched on
				uneqs->switchOnIIA();
				calculationSuccessful = startTryCalc(localLastSuccessfulNode, node); //**************************************
			}
			else
			{ // start with IIA uneqs switched off

				IO::println("Try a first calculation with iia switched off....");
				uneqs->switchOffIIA();
				// store the original node
				Node *originalNode = new Node(node->nodeType);
				originalNode->clone(node);

				// do a first calculation
				calculationSuccessful = startTryCalc(localLastSuccessfulNode, node); //**************************************

				if (calculationSuccessful)
				{
					IO::println("First calculation was successful!");
					IO::println("Repeat calculation with iia switched on..");
					uneqs->switchOnIIA();
					calculationSuccessful = startTryCalc(localLastSuccessfulNode, node); //**************************************
					if (calculationSuccessful)
					{
						IO::println("This was successful!!");
					}

					// repeat calculation with the original node
					// with updated start estimations
					copyUnknowns(node, originalNode);
					calculationSuccessful = startTryCalc(localLastSuccessfulNode, originalNode); //**************************************
					node->clone(originalNode);

				}
				else
				{
					IO::println("First calculation was not successful");
				}

				delete originalNode;

			} 
		}
		else
		{ // not first calculation
			calculationSuccessful = startTryCalc(localLastSuccessfulNode, node); //**************************************
		}

		if (calculationSuccessful && trying)
		{
			IO::println("Trying better start estimations was successful!!");
		}

		trying = false;

		if (flag->isCancelled())
		{
			// do nothing
		}
		else if ((!calculationSuccessful))
		{

			// we should copy the original node into node again!

			if (stopIfFailed)
			{
		// The calculation was not successful, so do it again with the original input
				// and write log file

			  //  try{
					//IO.showMessage("dumping cell to disk");
					// file should exist
			   //    orgNode.dumpCell();
				//}catch (IOException | ReadException e){
					//e.printStackTrace();
				//}


				//boolean successAfterAll = false;
				uneqs->monitor = true; // indicates to write log file iteration.dat
				// may be we should open the iteration log file here
		        node->clone(orgNode);
				bool successAfterAll = localCalculate(node); //**************************************
				uneqs->monitor = false;
				// and close the log file here

				if (successAfterAll)
				{ // how can this happen? and should we report it?
					IO::println("*****************************");
					IO::println("*                           *");
					IO::println("* SUCCESS after all!!!!!!!! *");
					IO::println("*                           *");
					IO::println("*****************************");
				}
				else
				{

					throw (ReadException(name->name + ": Sorry, this calculation was not successful.\n Iteration report written to Iteration.dat"));
				}
			}
			if (exitIfFailed)
			{
				flag->pleaseStop("Calculator exit if failed ");
				throw (ExitException(name->name + ": Sorry, this calculation was not successful."));
			}

		}

		// set the failed variable in the node to indicate that calculation was successful or not
		if (failedIndex > 0)
		{
			if (calculationSuccessful)
			{
				node->setValue(failedIndex, 0);
			    //localLastSuccessfulNode->clone(node);
			}
			else
			{
				node->setValue(failedIndex, 1);
			}
		}

		totalNrIterVar->setValue(this->uneqs->getTotalNrIter());
		return calculationSuccessful;

	}

	bool Calculator::startTryCalc(Node *last_successful_node, Node *node)// throw(ParserException)
	{
		trylevel = 0;
		bool success = tryCalc(last_successful_node, node);
		if (success)
		{
			localLastSuccessfulNode->clone(node);
		}
		else
		{
			if (calculatorStopFlag->isCancelled())
			{
			   IO::println("Calculation interrupted!..");
			}
			else
			{
			   IO::println("Unfortunately this does not seem to work. Trylevel: " + std::to_string(trylevel));
			}
		}
		return success;
	}

	bool Calculator::tryCalc(Node *last_successful_node, Node *node)// throw(ParserException)
	{
		if (calculatorStopFlag->isCancelled())
		{
			return false;
		}

		trylevel++;

		// check whether we have to return from this recursive calculation
		if (trylevel > maxtry)
		{
			return false;
		}

		if (localCalculate(node))
		{
			/* new node was successful copy content of new_node to last_successful node */
			last_successful_node->clone(node);
			trylevel--;
			return true; // success
		}

		// We should first try to calculate the last successful node
		if (!localCalculate(last_successful_node)) {
			IO::println(name->name + ": Last succesful node failed!");
			return false;
		}



		// The calculation was not successful, so try to improve start estimations.
		trying = true;
		Node *originalNode = node->clone();


		if (trylevel == 1)
		{
			IO::println(name->name + ": Improving start estimations:.");
		}
		else if (trylevel > 1)
		{
			IO::print(".");
		}

		Node *interimNode = new Node(last_successful_node->nodeType);

		interimNode->clone(last_successful_node);

		//double f = 0.05; //20 steps
		//int m = nrIntermediateNodes;
		int m = 30;
		for (int n = 0; n <= m; n++)
		{
			interimNode->nodeBetween(last_successful_node, node, (static_cast<double>(n) / static_cast<double>(m)));
			// copy the unknown values from the last successful iteration to this node
			copyUnknowns(last_successful_node, interimNode);
			// try to calculate this intermediate node
			if (!tryCalc(last_successful_node, interimNode))
			{
				delete interimNode;
				return false;
			}
			IO::print("&");
		}
		IO::println("|");

		node->clone(interimNode);
		trylevel--;

		delete interimNode;
		return true;
	}

	bool Calculator::localCalculate(Node *node)// throw(ParserException)
	{
		// copy variables from this node at the time
		iob1->copyToLocal(node);

		if (!optimized)
		{
			//long long starttime = System::currentTimeMillis();
			auto t0 = high_resolution_clock::now();
			uneqs->initialise();
			IO::println("Parsing expressions of " + name->name + "..... ");
			expressions->initialize();
			IO::print("Optimizing expressions of " + name->name + "..... ");
			
			variables->optimizeExpressions(expressions->parser);
			
			auto t1 = high_resolution_clock::now();
			auto duration = duration_cast<milliseconds>(t1 - t0).count();
			IO::print(StringHelper::toString((double)duration / 1000.0));	IO::println(" sec.");

			//variables->initializeParentsArrays(); Not necessary in C++
			IO::println(std::to_string(variables->getNrVariables()) + " variables, " + std::to_string(expressions->nrExpressions) + " expressions, " + std::to_string(uneqs->nrActiveUneqs) + " equations.");
			optimized = true;
		}

		bool success = uneqs->iterate(calculatorStopFlag);
		totNrIterations += uneqs->getTotalNrIter();
		totalNrIterVar->setValue(totNrIterations);
		nrIterVar->setValue(uneqs->getNrIter());

		//do we copy results to global node if not successful?
		if (success)
		{
			iob1->copyToGlobalFromCalculator(node);
		}
		return success;
	}

	void Calculator::copyUnknowns(Node *from, Node *to)
	{
		for (auto tmp : uneqs->uneqs)
		{
			int n = from->nodeType->index(tmp->unknown->name);
			if (n == -1)
			{
				IO::println("Warning: " + tmp->unknown->name + " has not been defined as cell variable!");
			}
			to->setValue(n, from->getvalue(n));
		}
	}

	std::vector<Var*>* Calculator::getGlobalVariables()
	{
		return variables->getGlobalVariables();
	}

	std::vector<std::string>* Calculator::getVariableNames()
	{
		return variables->getVariableNames();
	}
}
