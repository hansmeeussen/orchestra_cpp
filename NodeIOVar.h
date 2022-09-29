#pragma once
#include <string>
#include "OrchestraException.h"
#include "Var.h"

namespace orchestracpp
{
	class NodeIOVar : public Var
	{

	public:
		NodeIOVar(OrchestraReader *in) /*throw(ReadException)*/;

		NodeIOVar(const std::string &n, double i);

		NodeIOVar(const std::string &n);

		virtual void setExtValue(double value);

		virtual double getExtValue();
	};


	class NodeIOVarConstantFactor : public NodeIOVar
	{

	public:
		double factor = 1;

		NodeIOVarConstantFactor(const std::string &n, double i);

		NodeIOVarConstantFactor(const std::string &n);

		void setExtValue(double value) override;

		double getExtValue() override;

		virtual void setFactor(double f);
	};

}
