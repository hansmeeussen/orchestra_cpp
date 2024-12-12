#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "OrchestraException.h"
#include "Var.h"
#include "Parser.h"

namespace orchestracpp { class Var; }
namespace orchestracpp { class OrchestraReader; }
namespace orchestracpp { class ReadException; }
namespace orchestracpp { class Parser; }

namespace orchestracpp
{

	/**
	 * A VarGroup object manages the group of variables within a Calculator
	 *
	 * Checked Dec 2018
	 * @author Hans Meeussen
	 */
	class VarGroup
	{


	private:
        std::unordered_set <Var*>variables; // the variables, no duplicates, not ordered, so order not the same as Java  version!
		std::map <std::string, Var*> variableIndx; // the variables by name 
		std::vector<std::string> allVariableNames;
		//std::vector<std::string> alphabeticVariableNames;


	public:
		std::vector<Var*> globalVariables;   // Variables that the calculator would like to see stored globally in nodes
		std::unordered_map <std::string, std::string> synonyms;

		virtual ~VarGroup()
		{
			//Todo implement to delete variables;
			//delete globalVariables;
		}

		VarGroup() {}

		virtual Var *readOne(OrchestraReader *in)/* throw(ReadException, IOException)*/;

		virtual void createSynonym(OrchestraReader *in) /*throw(ReadException, IOException)*/;

		// Get a variable by its name or synonym.
		virtual Var *get(const std::string &name);

		// This list includes synonyms
		// we use a Vector because this collection is used in user interface swing objects
		virtual std::vector<std::string>* getVariableNames();

		virtual void optimizeExpressions(Parser* parser);

		virtual void setDependentMemoryNodes();

		//virtual void initializeParentsArrays();

		virtual int getNrVariables();

		virtual void addToGlobalVariables(Var *var);

		virtual void addToGlobalVariables(const std::string &varname);

		virtual std::vector<Var*>* getGlobalVariables();

		virtual std::unordered_map <std::string, std::string>* getSynonyms();

//**  only used for testing
//		virtual std::string getVariableNamesLine();

//		virtual std::string getVariableValuesLine();




	};

}
