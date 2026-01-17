#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "FileBasket.h"



namespace orchestracpp
{
	class Calculator;

	/**
	 * The order in which nodes and calculators should be defined is as
	 * follows: 1) first the node type should be defined with Node.createType() this
	 * reads the (static) node variables with their default values 3) create the
	 * nodes, after the first node is created, it is not possible to add more
	 * variables 4) link the calculators to the node type
	 *
	 */

	/**
	 * The Node class represents a single node, or cell, of a transport system. It
	 * contains the set of state variables per cell. The static variables and
	 * methods are constant for all nodes of the system.
	 */
	class NodeType final
	{


	public:
		std::vector<std::string> names;    // names of node variables
		std::vector<std::string> outputVariables;;
		std::vector<double> defaultValues; // default values for node variables
		std::unordered_map<std::string, std::string> varSource; //= std::unordered_map(); // the source of the variable definition
		int nrVars   = 0; // number of node variables
		int nrStatic = 0; // number of static node variables

		bool isInitialised = false; // Indicates whether nodeType is initialised can be called from different threads
		double *staticValues = nullptr; // Static node variables

		bool noMoreAddingVariables = false;

		std::unordered_map<std::string, int> indices;
		//std::map<std::string, int> indices;

		std::unordered_map<std::string, std::string> synonyms; // a list of synonyms

		~NodeType()
		{
			delete[] staticValues;
		}

		NodeType();

		void addVariable(const std::string &name, double value, bool is_static, const std::string &source);

		void addSynonym(const std::string &synonym, const std::string &name);

		void updateSynonyms();

		int index(const std::string &name);

		//int oldindex(const std::string &name);

		bool isVariablePresent(const std::string& name);

		/**
		 * Read a node type definition from a file. Static (1 value per node type)
		 * and ordinary (1 value per node) variables with their initial values. Var:
		 * pH pe time Data: 7 5 0
		 *
		 * @Var: pH 7
		 * @Var: pe 5
		 */
		bool verbose = false;

		//NodeType(FileID *filename) throw(ReadException); // call the default constructor

		void readGlobalVariablesFromFile(FileBasket *fileBasket, const std::string &filename) /*throw (IOException)*/;

		void readGlobalVariablesFromOutputFile(FileBasket* fileBasket, const std::string& filename);

		/**
		 * This should not change existing variables
		 */
		void useAllGlobalFileVariables(std::vector<std::string> filenames, FileBasket *fileBasket) /*throw (ReadException)*/;

		/**
		 * Only add variables that do not exist. Existing variables are not changed.
		 */
		void useGlobalVariablesFromCalculator(Calculator *calculator);

		void initialise();

		int getNrVars();

		std::string getName(int i);
	};

}
