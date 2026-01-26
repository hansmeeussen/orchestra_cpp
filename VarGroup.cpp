//#include "pch.h"
#include "VarGroup.h"
#include "Var.h"
#include "OrchestraReader.h"
#include "IO.h"

namespace orchestracpp
{

	Var* VarGroup::readOne(OrchestraReader* in)// throw(ReadException, IOException)
	{
		std::string name = in->readWord();
		double value = in->readDouble();

		return addVariable(name, value);
	}


	Var* VarGroup::addVariable(std::string name, double value)// throw(ReadException, IOException)
	{
		Var* tmp = get(name);

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
		synonyms.emplace(synonym, variableName);
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

		// We now have all expressions optimized and dpendent memory nodes set
		// To optimize specifically this C++ version we replace a series of connected
		// PlusNodes by a single MultiPlusNode. This reduces the number of virtual method pointers that
		// have to be looked up in memory by ca 50%, and  reduces overall runtime by ca 25%
        // In java this optimization is not faster, most likely because the Java virtual machine 
		// ptimizes this automatically at run time 
		//*
        for (auto v : variables){

			if (v->memory != nullptr) {

				if ((typeid(*v->memory->child) == typeid(PlusNode))) {
					std::vector<PlusNode*>* plusNodes = new std::vector<PlusNode*>;
					((PlusNode*)v->memory->child)->findMultiPlusNode(plusNodes);
				
				    if (plusNodes->size() >= 2) {

						// now we replace the original PlusNode pointer of this variable to the new MultiPlusNode 
					    v->memory->child = new MultiPlusNode(plusNodes, (PlusNode*)v->memory->child);
				    }
					// and we can delete the plusNodes as these are only used during intialisation
					// delete(plusNodes);
				}
				else {
					

				}
	        }
        }
		//*/

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

	std::unordered_map <std::string, std::string>* VarGroup::getSynonyms() {
		return &synonyms;
	}


	std::string VarGroup::getVariableNamesLine()
	{
		std::string line;

		// create alphabethic vector of variable names
		// extremely inefficient, but just used for testing
		std::vector<std::string> alphabeticVariableNames;
		for (auto v : variables) {
			alphabeticVariableNames.push_back(v->name);
		}
		std::sort(alphabeticVariableNames.begin(), alphabeticVariableNames.end()); 

        for (auto n : alphabeticVariableNames) {
			Var* v = get(n);
            line += IO::format(v->name, 30);
            line += '\t';
         }

		return line;
	}

	std::string VarGroup::getVariableValuesLine()
	{
		// create alphabethic vector of variable names
		// extremely inefficient, but just used for testing
		std::vector<std::string> alphabeticVariableNames;
		for (auto v : variables) {
			alphabeticVariableNames.push_back(v->name);
		}
		std::sort(alphabeticVariableNames.begin(), alphabeticVariableNames.end());

		std::string line;

		for (auto n : alphabeticVariableNames) {
			Var* v = get(n);
			line += IO::format(v->getValue(), 30, 12);
            line += IO::format("\t", 2);
		}

		return line;

	}

}
