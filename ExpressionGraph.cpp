#include "ExpressionGraph.h"
#include "VarGroup.h"
#include "Parser.h"
#include "OrchestraReader.h"
#include "OrchestraException.h"
#include "ExpressionString.h"
#include <vector>

namespace orchestracpp
{

	ExpressionGraph::ExpressionGraph(VarGroup *variables)// throw(IOException)
	{
	   //this->variables = variables;
	   parser = new Parser(variables);

	}

	void ExpressionGraph::readExpression(OrchestraReader *infile)// throw(ParserException)
	{
	   try
	   {
		  std::string line = infile->readLine(); // the expression line (without spaces and tabs)
		  //format: (<stage>,"<variable>=<expresssion>")

		  int openingBracketPosition = (int)line.find('(');
		  if (openingBracketPosition < 0)
		  {
			  throw ParserException("Could not find starting bracket in expression line: \n" + line);

		  }

		  int startExpressionPosition = (int)line.find('"');
		  if (startExpressionPosition < 0)
		  {
			  throw ParserException("Could not find start of expression \" in line: \n" + line);
		  }

		  int endExpressionPosition = (int)line.rfind('"');
		  if (startExpressionPosition == endExpressionPosition)
		  {
			 throw ParserException("End of expression \" was not found in line: \n" + line);
		  }


		  addExpression2(line[openingBracketPosition + 1], line.substr(startExpressionPosition + 1, endExpressionPosition - (startExpressionPosition + 1)));
	   }
	   catch (const IOException &ioe)
	   {
	   }

	}

	void ExpressionGraph::addExpression2(char stage, const std::string &expressionInputString)// throw(ParserException)
	{
  	   ExpressionString* tempVar = new ExpressionString(expressionInputString);
	   addExpression(stage, tempVar);
	}

	void ExpressionGraph::addExpression(char stage, ExpressionString *expression)// throw(ParserException)
	{
	   //nrExpressions++;
	   switch (stage)
	   {
		  case '1':
		  {
			 stage1exp.push_back(expression);
			 break;
		  }
		  case '2':
		  {
			 stage2exp.push_back(expression);
			 break;
		  }
		  case '3':
		  {
			 stage3exp.insert(stage3exp.begin(),expression);
			 break;
		  }
		  case '4':
		  {
			 stage4exp.insert(stage4exp.begin(), expression);

			 break;
		  }
		  case '5':
		  {
			 stage5exp.insert(stage5exp.begin(), expression);
			 break;
		  }
		  default:
		  {
			 throw ParserException("Expression stages should be 1, 2, 3, 4, or 5");
		  }
	   }
	}

	int ExpressionGraph::initialize()// throw(ParserException)
	{
	   int nrExpressions = 0;

	   for (auto const &exp : stage1exp) {
		   parser->addExpression(exp);
		   nrExpressions++;
	   }
	   for (auto exp : stage2exp) {
		   parser->addExpression(exp);
		   nrExpressions++;
	   }
	   for (auto exp : stage3exp) {
		   parser->addExpression(exp);
		   nrExpressions++;
	   }
	   for (auto exp : stage4exp) {
		   parser->addExpression(exp);
		   nrExpressions++;
	   }
	   for (auto exp : stage5exp) {
		   parser->addExpression(exp);
		   nrExpressions++;
	   }

	   return nrExpressions;

	}
}
