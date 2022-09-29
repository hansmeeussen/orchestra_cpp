//#include "pch.h"
#include "VarGroup.h"
#include "Var.h"
#include "OrchestraReader.h"
#include "IO.h"

namespace orchestracpp
{

	Var *VarGroup::readOne(OrchestraReader *in)// throw(ReadException, IOException)
	{
		std::string name = in->readWord();
		double value = in->readDouble();

		Var *tmp = get(name);

		if (tmp == nullptr)
		{ // variable does not exist so create new one
			tmp = new Var(name, value);
			variables.insert(tmp);
			variableIndx[name] = tmp;			
		}
		else
		{
			// if a variable is defined more than once, should we overwrite its value?, yes 
			tmp->setValue(value);
		}
		return tmp;
	}

	void VarGroup::createSynonym(OrchestraReader *in) //throw(ReadException, IOException)
	{
		std::string synonym = in->readWord();
		std::string variableName = in->readWord();
		Var *tmpVariable = get(variableName);
		if (tmpVariable == nullptr)
		{
			throw ReadException("Could not find variable: " + variableName + " to create synonym!");
		}
		variableIndx.emplace(synonym, tmpVariable);
	}

	Var* VarGroup::get(const std::string &name)
	{
		return variableIndx[name];
	}

	// in contrast with the java version we do not create a new vector here 
	
	std::vector<std::string> * VarGroup::getVariableNames()
	{
		allVariableNames.clear();
		for (auto v : variableIndx) {
			allVariableNames.push_back(v.first); // iterating over a map gives pairs
		}

		return &allVariableNames;
	}

	void VarGroup::optimizeExpressions(Parser* parser)
	{
		for (auto v : variables){
			v->optimizeExpression(parser);
		}

		setDependentMemoryNodes();
	}

	void VarGroup::setDependentMemoryNodes() {
		for (auto v : variables) {
			if (!v->constant()) {
				if (v->memory != nullptr) {
					v->setDependentMemoryNodes();
				}
			}
		}

// not necessary in C++
//		for (auto v : variables) {
//			v->initializeDependentMemoryNodesArray();
//		}
	}

//	void VarGroup::initializeParentsArrays()
//	{
		// not necessary in c++
//		 for (auto v : variables){
//		     if (v->usedForIO){
//			    v->initializeDependentMemoryNodesArray();
//		     }
//		 }
//	}

	int VarGroup::getNrVariables()
	{
		return variables.size();
	}

	void VarGroup::addToGlobalVariables(Var *var)
	{
		globalVariables.push_back(var);
	}

	void VarGroup::addToGlobalVariables(const std::string &varname)
	{
		Var* tmp = get(varname);
		if (tmp != nullptr) {
			globalVariables.push_back(get(varname));
		}
	}

	std::vector<Var*>* VarGroup::getGlobalVariables()
	{
		return &globalVariables;
	}

	std::string VarGroup::getVariableNamesLine()
	{
		std::string line;
		for (auto v : variables) {
			line += IO::format(v->name, 20);
			line += '\t';
		}
		return line;
	}

	std::string VarGroup::getVariableValuesLine()
	{
		std::string line;
			for (auto v : variables) {
			line += IO::format(v->getValue(), 20, 5);
			line += IO::format("\t", 2);
		}
		return line;

	}

}
