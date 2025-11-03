#pragma once

#include "ExpressionNode.h"
#include <string>
#include <unordered_set>
#include "OrchestraException.h"
#include "OrchestraReader.h"
#include "MemoryNode.h"
#include "Parser.h" 

namespace orchestracpp
{

	/**
	 * Variables are used for the calculations in Calculator objects. Each variable
	 * for which the value is calculated contains a link to its expression tree, and
	 * evaluates this if necessary.
	 *
	 * @author Hans Meeussen
	 */
	class Var : public ExpressionNode
	{

	private:
		double value = 0;
	public:
		MemoryNode *memory = nullptr;
	//	std::vector<MemoryNode*> newDependentMemoryNodes;
		// a set automatically prevents duplicate values
		std::unordered_set<MemoryNode*> dependentMemoryNodes;
		
	public:   
		std::string name;
		bool isConstant = true; // this variable is a constant
		bool isEquation = false; // this variable is used as equation output/input
		bool isUnknown  = false;
		bool usedForIO  = false; // this variable is used for IO between cell and calculator

		bool immutable = false;
	
		bool usedAsExpressionResult = false;
		// public boolean usedAsExpressionInput  = false;
		// we should only allow variables to be used as result until they are
		// used as input.
		// so once used they become immutable
		
		// what happens if a variable is used for io, but not as expression input or expression result?

		virtual ~Var()
		{
		}


		Var(OrchestraReader *in) /*throw(ReadException)*/;

		Var(const std::string &n, double i);

		Var(const std::string &n);

		/**
		 * Set the value of this variable and notify all dependent variables and
		 * expressions that re-evaluation is necessary.
		 */
		virtual void setValue(double value);

		/**
		 * Get the value for this variable and recalculate if necessary
		 */
		virtual double getValue();

		virtual double getIniValue();

		virtual void setConstant(bool flag);


		/*
		 * The expression node methods
		 */
		double evaluate() override;

		bool constant() override;

		virtual std::string toString();

		ExpressionNode *optimize(Parser* parser) override;

		/**
		 * Add an ExpressionNode to the list of expression nodes that are dependent
		 * on this variable
		 */
		void setDependentMemoryNode(MemoryNode *dependentMemoryNode) override;

		//virtual void initializeDependentMemoryNodesArray();

		virtual void optimizeExpression(Parser* parser);

		virtual void setDependentMemoryNodes();

	};

}
