#pragma once

#include <string>
#include <vector>
#include "stringhelper.h"
#include "OrchestraException.h"
#include "OrchestraReader.h"
#include "ParameterList.h"
#include "OObjectPieceOfText.h"

namespace orchestracpp
{

	class OObject
	{

	private:
		std::string name;
		ParameterList *placeholders = nullptr; // the list of placeholders
		std::string bodytext;
		std::string documentation;
		std::vector<OObjectPieceOfText*> textPointers; // contains pieces of text
		std::vector<OObjectPieceOfText*> placeHolderPointer;
		std::vector<OObjectPieceOfText*> deleteableTextPointers;


		// Static methods to read members from reader
	public:
		virtual ~OObject()
		{
			delete placeholders;

		   //delete textpointers
			for (auto o : textPointers) {
				delete o;
			}

			// delete placeholderpointers
			for (auto o : placeHolderPointer) {
				delete o;
			}



		}

		static std::string readObjectName(OrchestraReader *in) /*throw(IOException)*/;

		static ParameterList* readParameterList(OrchestraReader* in);

		static std::string readDocumentation(OrchestraReader *in) /*throw(IOException)*/;

		/**
		 * @param in the input orchestraReader
		 * @return a string containing textblock between {} The starting { needs to
		 * be removed from the Reader (is not expected to be present in the reader)
		 * The closing } will be removed from the block.
		 * @throws IOException
		 */
		static std::string readBlock(OrchestraReader *in) /*throw(IOException)*/;

		static std::string readBodytext(OrchestraReader *in)/* throw(IOException)*/;

		// Non static methods ----------------------------------------------------------
		OObject(const std::string &name);

		OObject(const std::string &name, ParameterList *placeholders, const std::string &documentation, const std::string &bodytext);

		virtual std::string getName();

		virtual std::string getKey();

		virtual std::string getIdentifier();

		virtual std::string getPlaceholders();

		virtual std::string getDocumentation();

		virtual std::string getBodytext();

		virtual int getNrParam();

		virtual bool identifierOK(const std::string &name, int nrp);

		virtual void append(OrchestraReader *in) /*throw(IOException)*/;

		virtual void insert(OrchestraReader *in) /*throw(IOException)*/;

		/**
		 * Initialisation of textpointers and placeholderpointers, occurs only once
		 */
		virtual void initialiseTextPointers();

		virtual std::string getSubstitutedBodytext(ParameterList *parameters);

	};

}
