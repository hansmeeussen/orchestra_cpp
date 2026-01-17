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

	class OObject final
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

		~OObject();

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

		std::string getName();

		std::string getKey();

		std::string getIdentifier();

		std::string getPlaceholders();

		std::string getDocumentation();

		std::string getBodytext();

		int getNrParam();

		bool identifierOK(const std::string &name, int nrp);

		void append(OrchestraReader *in) /*throw(IOException)*/;

		void insert(OrchestraReader *in) /*throw(IOException)*/;

		/**
		 * Initialisation of textpointers and placeholderpointers, occurs only once
		 */
		void initialiseTextPointers();

		std::string getSubstitutedBodytext(ParameterList *parameters);

	};

}
