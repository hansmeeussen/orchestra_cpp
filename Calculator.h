#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "OrchestraException.h"
#include "stringhelper.h"
#include "ExpressionGraph.h"
#include "VarGroup.h"
#include "Node.h"
#include "NodeType.h"
#include "StopFlag.h"
#include "UnEqGroup.h"
#include "NodeIOObject.h"

namespace orchestracpp
{

	/**
	 * All calculations in Orchestra are performed by Orchestra Calculator objects.
	 * A Calculator is "programmed" by its own input file, in terms of Variables,
	 * Expressions and UnEq's (Unknown-Equation combination). 
	 * A standard Calculator operates on 1 node (e.g chemical equilibrium calculators, update mass). 
	 * A transport Calculator operates on 2 nodes (Convection, diffusion calculators) and has acces to the variables 
	 * in both nodes. 
	 * Data communication between calculators and nodes is performed by the IOObjects, which
	 * are programmed during the linking stage.
	 * Linking takes place when the calculator perform method is used  for the first time on a node.
	 *
	 * @author Hans Meeussen, (1995 C++) (1999 Java) (2020 C++ again!)
	 */
	class Calculator
	{

	// Define variables that are visible to other threads (via clone) as volatile!
	public:
		FileID *name                 = nullptr;
		bool switchOnIIA             = false;
		StopFlag *calculatorStopFlag = nullptr;

	protected:

		std::string  expandedText; // may be we should zip this one?
		VarGroup          *variables = nullptr;
		ExpressionGraph *expressions = nullptr;
		UnEqGroup             *uneqs = nullptr;

		bool optimized = false;

		NodeIOObject *iob1            = nullptr;
		Node *localLastSuccessfulNode = nullptr; // should we store this one not locally, but in the calculator group? or threadgroup?

	private:
		int trylevel = 0;
		int maxtry = 10;
		int nrIntermediateNodes = 100;
		bool trying = false;
		bool stopIfFailed = false;
		bool exitIfFailed = false;
		double totNrIterations = 0;

		Var *nrIterVar      = nullptr;
		Var *totalNrIterVar = nullptr;

		int failedIndex = -100; // will be overwritten at initialisation
		int nodeIDIndex = -101;
		Node *orgNode = nullptr;
		bool silent = false;

	public:
		virtual ~Calculator()
		{
			delete variables;
			delete expressions;
			delete uneqs;

			delete iob1;
			//delete localLastSuccessfulNode;
			//delete calculatorStopFlag;
			//delete orgNode;
		}

		Calculator();

		Calculator(const std::string filename);
			
		Calculator(FileID* name) /*throw(ReadException)*/;

		Calculator(FileID* name, std::string extraText);
	protected:
		Calculator(const Calculator *oldCalculator);// throw(ReadException);

	public:
		virtual Calculator *clone();

	protected:
		virtual void readSystemFromExpandedInput(const std::string &text)/* throw(IOException, ReadException)*/;

		virtual void readVariablesAndExpressions(OrchestraReader *infile)/* throw(ReadException, IOException)*/;

		/**
		 * This is the highest level calculate method that is called externally
		 * Normally it would not be necessary to synchronize this method, but just in case...
		 */
	public:
		virtual bool calculate(Node *const node, StopFlag *flag)/* throw(ReadException, ParserException, ExitException)*/;

		/**
		 * This recursive method tries to calculate a node with a calculator it uses
		 * the last successful node and the new node as input. It updates the last
		 * successful node if successful. If the calculations fails, it recursively
		 * tries to improve the estimations of the unknowns by calculating nodes in
		 * between the last successful node and the new node and using the results
		 * of successful calculations as new estimates.
		 *
		 * Hans Meeussen 30 November 1999.
		 */


		/**
		 * We should make sure that a successful node is only calculated once!
		 */
	private:
		virtual bool startTryCalc(Node *last_successful_node, Node *node)/* throw(ParserException)*/;

		virtual bool tryCalc(Node *last_successful_node, Node *node)/* throw(ParserException)*/;

	protected:
		virtual bool localCalculate(Node *node) /*throw(ParserException)*/;

		/**
		 * copy the unknown variables from one node to the other
		 * Nodes should both be from the same node type!
		 * Actually the calculator also belongs to a certain node type!
		 */
	public:
		virtual void copyUnknowns(Node *from, Node *to);

		virtual std::unordered_map <std::string, std::string>* getSynonyms();

		virtual std::vector<Var*>* getGlobalVariables();

		virtual std::vector<std::string>* getVariableNames();
	};

}
