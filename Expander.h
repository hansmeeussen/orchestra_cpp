#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "stringhelper.h"
#include "OrchestraReader.h"
#include "OObjectBasket.h"
#include "FileBasket.h"


namespace orchestracpp
{

	/**
	 * Expander The Expander is the ORCHESTRA text pre-processor that recursively processes
	 * input text files. 
	 * It enables the defition of "objects" which basically are parameterised pieces of text.
	 * that are expanded when an input file is read by an expander
	 *
	 * The expander can carry out the following tasks: 
	 * - Define a class (or object)
	 * - Insert text to existing class (append, insert) Include a file Delete a class
	 * - Write expanded file 
	 * - Write unexpanded file 
	 * - Expand class/object reference. * (replace object reference with expanded text)
	 * 
	 * Todo: make the expander compatible with .xml files.
	 * How:  currently the syntax of the expander uses <> characters  which conflicts with xml syntax
	 * - this was done in 2018
	 *
	 * @author Hans Meeussen 1995 (C++) January 1999 converted to Java. (Aberdeen, 5
	 * Abbotshall Drive)
	 *
	 * Added "warn" "nowarn" option January 2013 
	 * Updated to Java 7 Switch statement 6 May 2014
	 * Introduced "static" sharedBasket 3/11/2016
	 */
	class Expander
	{
		// this objectbasket is shared by all expanders.
		// we have to see whether this is the desired behaviour!
		// should we not clean the shared basket 
	private:
		//static const OObjectBasket* sharedBasket;// = new OObjectbasket();


	public:
		OObjectBasket basket2; // contains all the orchestra objects

		FileBasket *fileBasket = nullptr; // the default file basket that is used for including files.
		bool logactivities     = false; // just a flag that can be set with  the keyword @logactivities:

		std::string separatorChar{"//"};

	private:
		bool warnForUndefinedObjects = false;
		std::vector<std::string> nowarnObjects;


	public:

		Expander()
		{
			// calculator keywords
			// This is initialised for new instance
			nowarnObjects.push_back("var:");
			nowarnObjects.push_back("calc:");
			nowarnObjects.push_back("uneq:");
			nowarnObjects.push_back("uneq2:");
			nowarnObjects.push_back("uneq3:");
			nowarnObjects.push_back("stop:");
			nowarnObjects.push_back("synonym:");
			nowarnObjects.push_back("globalvar:");
			nowarnObjects.push_back("global:");
			nowarnObjects.push_back("output:");
			nowarnObjects.push_back("output");
			nowarnObjects.push_back("maxtry:");

			// user interface keywords
			nowarnObjects.push_back("database:");

			// objects that are only defined during the second expansion phases of a calculator
			nowarnObjects.push_back("link");
			nowarnObjects.push_back("entity");
		}

		virtual ~Expander()
		{
			//delete basket; // we have to be careful with this
		}

		static std::string expandString(const std::string& inputstring)/* throw(ReadException)*/;

		static std::string expandString(const std::string &inputstring, Expander& expander)/* throw(ReadException)*/;

		void expand(OrchestraReader*, Writer*, FileBasket*) /*throw(ReadException)*/;

		/**
		 * transpose a text containing output/input data
		 * @return 
		 */
        // not (yet)implemented in the C++ version
		//static std::string transpose(const std::string &inputText);

	};

}
