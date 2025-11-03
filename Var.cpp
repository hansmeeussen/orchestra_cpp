//#include "pch.h"
#include "Var.h"
#include "MemoryNode.h"
#include "OrchestraReader.h"
#include "OrchestraException.h"
#include "Parser.h"


namespace orchestracpp
{

	Var::Var(OrchestraReader *in)// throw(ReadException)
	{
		try
		{
			name  = in->readWord();
			value = in->readDouble();
		}
		catch (const ReadException &re)
		{
			std::string tmp = "";
			tmp = tmp + re.what() + "The definition: @Var: " + name + " is not correct! ";
			throw ReadException(tmp);
		}
		catch (const IOException &f)
		{
			std::string tmp = "";
			tmp = tmp + f.what() + "The definition: @Var: " + name + " is not correct! ";
			throw ReadException(tmp);
		}

	}

	Var::Var(const std::string &n, double i)
	{
		name = n;
		value = i;
	}

	Var::Var(const std::string &n) : Var(n, 1.111111111e-111)
	{
	}

	void Var::setValue(double value)
	{
		if (this->value == value)
		{
			return; // value was not changed
		}

		this->value = value;

//		for (auto n : newDependentMemoryNodes) {
//			n->needsEvaluation = true;
//		}

		for (auto n : dependentMemoryNodes) {
			n->needsEvaluation = true;
		}


	}

	double Var::getValue()
	{
		if (memory == nullptr)
		{
			return value;
		}
		else
		{
			return memory->evaluate();
		}
	}

	double Var::getIniValue()
	{
		return value;
	}

	void Var::setConstant(bool flag)
	{
		isConstant = flag;
	}

	double Var::evaluate()
	{
		return value;
	}

	bool Var::constant()
	{
		return isConstant;
	}

	std::string Var::toString()
	{
		return (name);
	}

	ExpressionNode *Var::optimize(Parser* parser)
	{
		if (constant())
		{
			return NumberNode::createNumberNode(evaluate(), parser);
		}
		return this;
	}

	void Var::setDependentMemoryNode(MemoryNode *dependentMemoryNode)
	{
		// check if we already have this one in the list and return if so
	//	for (auto n : newDependentMemoryNodes) {
	//		if (n == dependentMemoryNode) {
	//			return;
	//		}
	//	}
		
	//	newDependentMemoryNodes.push_back(dependentMemoryNode);


		// we use a set now that automatically prevents duplicate entries
		dependentMemoryNodes.insert(dependentMemoryNode);
	}

//	void Var::initializeDependentMemoryNodesArray()
//	{
//		// this is not necessary in C++
//	}

	void Var::optimizeExpression(Parser* parser)
	{
		if (memory != nullptr)
		{
//			memory->nrReferences++;
			memory->child = memory->child->optimize(parser);
//			memory->child->setDependentMemoryNode(memory);
		}
	}

	void Var::setDependentMemoryNodes()
	{
		if (memory != nullptr)
		{
			memory->child->setDependentMemoryNode(memory);
		}
	}
}
