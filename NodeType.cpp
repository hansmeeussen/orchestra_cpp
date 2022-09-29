#include "NodeType.h"
#include "IO.h"
#include "FileID.h"
#include "OrchestraException.h"
#include "OrchestraReader.h"
#include "FileBasket.h"
#include "Calculator.h"
#include "Var.h"
#include "stringhelper.h"

namespace orchestracpp
{

	NodeType::NodeType()
	{
		//names = std::vector(100);
		//defaultValues = std::vector(100);
		nrVars = 0;
		nrStatic = 0;
		isInitialised = false;

		// we add a standard NodeID variable for all nodes
		// its value is given in the nodePool, where the nodes are created
		// the index number for this variable should be 0
		// it is not a static variable
		addVariable("Node_ID", 0, false, "internal");

	}

	void NodeType::addSynonym(const std::string& synonym, const std::string& name) {
		synonyms.emplace(synonym, name);
		indices.emplace(synonym, index(name));
	}

	void NodeType::addVariable(const std::string &name, double value, bool is_static, const std::string &source)
	{
			if (isInitialised)
			{
				IO::println("It is not possible to add node variables after initialisation of node type!");
				return;
			}

			if (noMoreAddingVariables) {
				throw OrchestraException("Warning:: It is not possible to add more node variables after the index of a static node variable was asked!");
			}
    
			//if (index(name) == -1) // variable not present
			if(!isVariablePresent(name))
			{ // check if exists
				if (is_static)
				{ // static variables are added at the tail of lists
					names.push_back(name);
					defaultValues.push_back(value);
					varSource.emplace(name, source);
					nrStatic++;
				}
				else
				{ // normal variables are added at nrVars

					// normal variables should be inserted at nrVars
					// this means that all static variables should be moved one place up
					// it is so complicated to do this in C++ that we do this manually here.

					if (nrStatic > 0) {
						// so we first copy the last value one to the right 
						names.push_back(names[nrStatic + nrVars - 1]);
						defaultValues.push_back(defaultValues[nrStatic + nrVars - 1]);

						// now we move all static variables one place to the right
						for (int i = nrStatic + nrVars - 1; i > nrVars; i--) {
							names.at(i) = names.at(i - 1);
							defaultValues.at(i) = defaultValues.at(i - 1);
						}

						names[nrVars] = name;
						defaultValues[nrVars] = value;
						nrVars++;
					}
					else { // nrstatic == 0, so simply add name and value at the end of names and defaultvalues
						names.push_back(name);
						defaultValues.push_back(value);
						nrVars++;
					}

					varSource.emplace(name, source);
				}
				// re-create indices
				indices.clear();
				for (int i = 0; i < names.size(); i++) {
					indices.emplace(names[i], i);
				}

				// we also need to updat the synonym indices                        
				//synonyms.forEach((k, v)->indices.put(k, index(v)));


		        for (std::pair<std::string, std::string> item : synonyms) {
					std::string synonym = item.first;
					std::string name = item.second;
					indices.emplace(synonym, index(name));
				}


			}
			 
	}
	//*
	int NodeType::index(const std::string& name)
	{
		// check if indices contains name
		auto it = indices.find(name);
		if (it == indices.end()) {
			// name not found
			return -1;
		}
		else {
			//name found, return index
			int i = it->second;
			if (i >= nrVars) {
				// this was a static variable. 
				// Once we have asked the index of a static variable, we are not allowed to add more variables
				noMoreAddingVariables = true;
			}
			return i;
		}
	}

	bool NodeType::isVariablePresent(const std::string& name) {
		auto it = indices.find(name);
		if (it == indices.end()) {
			// name not found
			return false;
		}
		else {
			return true;
		}
	}

	void NodeType::readGlobalVariablesFromFile(FileBasket *fileBasket, const std::string &filename)// throw(IOException)
	{
		// we have to use an expanding filereader here, to include variables in include files and objects etc.
		FileID tempVar(fileBasket, filename);
		OrchestraReader ifile = *OrchestraReader::getExpandingFileReader(&tempVar);

		ifile.stripComment = true;

		while (!ifile.ready)
		{
			std::string word = StringHelper::toLower(ifile.readWord());

			if (word == "var:")
			{
				std::string Line = ifile.readLine();
				OrchestraReader stringIn(Line);

				/**
				 * add the variables to the dataset
				 */
				while (!stringIn.ready)
				{
					std::string word2 = stringIn.readWord();

					if (word2 == "}" || word2 == "")
					{
						break;
					}
					addVariable(word2, 0, false, filename); //**********************************
				}
				//					stringIn->close();
			}
			else if (word == "@var:")
			{
				word = ifile.readWord();
				addVariable(word, ifile.readDouble(), false, filename);
			}
		}
	}

	void NodeType::useAllGlobalFileVariables(std::vector<std::string> filenames, FileBasket *fileBasket)// throw(ReadException)
	{

		for (auto n: filenames)
		{
			try
			{
				readGlobalVariablesFromFile(fileBasket, n);
			}
			catch (const IOException &ioe)
			{
				IO::showMessage(ioe.what());
			}
		}
	}

	void NodeType::useGlobalVariablesFromCalculator(Calculator *calculator)
	{
		
		for (auto variable : *calculator->getGlobalVariables()) { // is this a list of strings?

			std::string name = variable->name;
			if (StringHelper::startsWith(name, "1.") || StringHelper::startsWith(name, "2.")) {
				name = name.substr(2); // is this correctly translated?
			}
			addVariable(name, variable->getIniValue(), false, calculator->name->name); // name points to a FileID

		}

	}

	void NodeType::initialise()
	{
			// create the static values array
			staticValues = new double[nrStatic];
    
			// fill array with default values
			// these start at the end of the default values array, so first iterate 
			// over all normal values
			std::vector<double>::const_iterator i = defaultValues.begin();
			for (int n = 0; n < nrVars; n++)
			{
				i++; // increase iterator to go to the next position in defaultvalues vector
			}
    
			for (int n = 0; n < nrStatic; n++)
			{
				staticValues[n] = *i;
				i++; // increase iterator to go to the next position in defaultvalues vector
				// or simply
				//staticValues[n] = defaultValues.at(nrVars + n);
			}
			isInitialised = true; 
	}

	int NodeType::getNrVars()
	{
		return nrVars;
	}

	std::string NodeType::getName(int i) {
		return names[i];
	}
}
