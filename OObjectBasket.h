#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include <vector>
#include "OrchestraException.h"
#include "stringhelper.h"
#include "OrchestraReader.h"

namespace orchestracpp { class OObject; }
namespace orchestracpp { class OrchestraReader; }

namespace orchestracpp
{

	/**
	 * The ObjecBasket class is a Collection that holds a set of OObject
	 * definitions. These definitions are used in the expansion process.
	 *
	 * @author Hans Meeussen 8/1998 1/9/2005 Improved performance by storing pieces
	 * of text 
	 * Updated structure to single map 4/11/2016
	 *
	 */
	class OObjectBasket
	{

	public:
		std::unordered_map<std::string, OObject*> newObjects;
		std::unordered_set<std::string> objectNames;

		/**
		 * read object from input reader and insert at start of list
		 */
		virtual ~OObjectBasket()
		{
			for (auto pair : newObjects) {
				delete pair.second;
    		}
		}

		virtual OObject *addObject(OrchestraReader *in) /*throw(IOException)*/;

		/**
		 * append text to existing object
		 */
		virtual OObject *append(OrchestraReader *in)/* throw(IOException)*/;

		/**
		 * insert text at start of bodytext
		 */
		virtual OObject *insert(OrchestraReader *in) /*throw(IOException)*/;

		/**
		 * Delete object with name and nr parameters read from in
		 */
		virtual OObject *ddelete(OrchestraReader *in) /*throw(IOException)*/;

		/**
		 * check if name is an existing objectname
		 */
		virtual bool isObjectName(const std::string &name);

		/**
		 * Return OObject with a given name and number of parameters
		 */
		virtual OObject *getObject(const std::string &name, int nrParam);

		virtual OObject *getObject(OrchestraReader *in) /*throw(IOException)*/;
	};

}
