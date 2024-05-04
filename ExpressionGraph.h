#pragma once

#include <string>
#include <vector>
#include "OrchestraException.h"
#include "VarGroup.h"
#include "Parser.h"

namespace orchestracpp
{

	/**
	 * The ExpressionGraph replaces old the relations_group class
	 *
	 * 1) add methods to the expression graph to set values of input variables
	 * The order in which the expressions are added to the expression graph is:
	 * 1) stage 1 same order as in inputfile
	 * 2) stage 2 same order as in inputfile
	 * 3) stage 3 in REVERSE order
	 * 4) stage 4 in REVERSE order
	 * 5) stage 5 in REVERSE order
	 * Hans Meeussen
	 * July 2002
	 */
	class ExpressionGraph
	{
    public:
	   //VarGroup *variables;

	   // a vector of expression string pointers
	   //std::vector<ExpressionString*> expressionStack[5]; // = new std::vector<std::string*>[5];

	  // std::vector<std::vector<ExpressionString*>> expressionStack; // = new std::vector<std::string*>[5];
	   std::vector<ExpressionString*> stage1exp;
	   std::vector<ExpressionString*> stage2exp;
	   std::vector<ExpressionString*> stage3exp;
	   std::vector<ExpressionString*> stage4exp;
	   std::vector<ExpressionString*> stage5exp;

	   Parser *parser = nullptr;

	   virtual ~ExpressionGraph()
	   {
		   delete parser;
	   }

	   ExpressionGraph(VarGroup *variables) /*throw(IOException)*/;

	   virtual void readExpression(OrchestraReader *infile) /*throw(ParserException)*/;

	   virtual void addExpression2(char stage, const std::string &expressionInputString) /*throw(ParserException)*/;

	   virtual void addExpression(char stage, ExpressionString *expression) /*throw(ParserException)*/;

	   //int nrExpressions = 0;

	   /**
	    * Add the expressions in the correct order to the expressionGraph
	    */
	   virtual int initialize()/* throw(ParserException)*/;

	};
}
