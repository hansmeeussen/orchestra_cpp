#include "NodeIOVar.h"

namespace orchestracpp
{

	NodeIOVar::NodeIOVar(OrchestraReader *in)/* throw(ReadException)*/ : Var(in)
	{
	}

	NodeIOVar::NodeIOVar(const std::string &n, double i) : Var(n, i)
	{
	}

	NodeIOVar::NodeIOVar(const std::string &n) : Var(n)
	{
	}

	void NodeIOVar::setExtValue(double value)
	{
		Var::setValue(value);
	}

	double NodeIOVar::getExtValue()
	{
		return Var::getValue();
	}

	NodeIOVarConstantFactor::NodeIOVarConstantFactor(const std::string &n, double i) : NodeIOVar(n, i)
	{
	}

	NodeIOVarConstantFactor::NodeIOVarConstantFactor(const std::string &n) : NodeIOVar(n)
	{
	}

	void NodeIOVarConstantFactor::setExtValue(double value)
	{
		NodeIOVar::setExtValue(value / factor);
	}

	double NodeIOVarConstantFactor::getExtValue()
	{
		return NodeIOVar::getExtValue() * factor;
	}

	void NodeIOVarConstantFactor::setFactor(double f)
	{
		factor = f;
	}
}
