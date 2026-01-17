#include "OObject.h"
#include "ParameterList.h"
#include "OObjectPieceOfText.h"
#include "IO.h"
#include "stringhelper.h"

namespace orchestracpp
{

	OObject::~OObject()
	{
		
		delete placeholders;
		
		// delete placeholderpointers
		for (auto o : placeHolderPointer) {
			delete o;
		}
		
	}

	std::string OObject::readObjectName(OrchestraReader *in)// throw(IOException)
	{
		// strip leading spaces and tabs
		std::string tempVar{' ', '\t'};
		in->readWhile(tempVar);
		// read name, do not remove end character
		std::string tempVar2{' ', '\t', '(', '\n', '\r', '{'};
		return in->readUntil(tempVar2);
	}

	ParameterList *OObject::readParameterList(OrchestraReader *in)
	{
		return new ParameterList(in);
	}

	std::string OObject::readDocumentation(OrchestraReader *in)// throw(IOException)
	{
		return in->readUntil("{");
	}

	std::string OObject::readBlock(OrchestraReader *in) //throw(IOException)
	{
		bool originalStripComment = in->stripComment;
		in->stripComment = false;
		std::string tmpText = "";
		int c;

		bool ready = false;
		do
		{
			c = in->read();
			switch (c)
			{
				case -1:
				case '}':
					ready = true;
					break; // so the final '}' is removed!
				case '{':  // we encounter a starting { and include this block recursively
					tmpText+=  ('{') + readBlock(in) + ('}');
					break;
		//---      
				// fixed a bug when s} was encountered by simply unread characters when 
				// reading after "sw
				case 's':
				case 'S': // check for sweep
				{
					tmpText +=  c; // first we write the "s"

					// check whether the rest of this word is "weep:"
					char input[5];

					char sweep[]  = { 'w', 'e', 'e', 'p', ':' };
					char sweep2[] = { 'W', 'E', 'E', 'P', '{' };

					bool issweep = true;

					for (int j = 0; j < 5; j++) { // check whether word is sweep:
						input[j] = in->read();
						if ((input[j] != sweep[j]) && (input[j] != sweep2[j]))
						{
							issweep = false;

							for (int i = 0; i <= j; i++) {
								in->unget();
							}

							break;
						}
					}

					if (issweep)
					{ // this was a sweep
						// replace Sweep: with Sweep{ and read block
						tmpText += ("weep{");
						tmpText += (readBlock(in));
						tmpText += ('}');
					}
					break;
				}
				default:
					tmpText += c;
					break;
			}
		} while (!ready);

		in->stripComment = originalStripComment;

		return tmpText;
	}

	std::string OObject::readBodytext(OrchestraReader *in)// throw(IOException)
	{
		in->readUntil("{"); // in case we do not read comment we start here
		in->readWhile("{"); // remove starting '{'

		std::string tmptext = readBlock(in); // without closing }
		if (StringHelper::startsWith(tmptext, "%") && StringHelper::endsWith(tmptext, "%"))
		{
			return tmptext.substr(1, (tmptext.length() - 1) - 1);
		}
		else
		{
			return tmptext;
		}
	}

	OObject::OObject(const std::string &name1) : name(name1), bodytext("")
	{
	}

	OObject::OObject(const std::string &name, ParameterList *placeholders, const std::string &documentation, const std::string &bodytext)
	{
		this->name = name;
		this->placeholders = placeholders;
		this->documentation = documentation;
		this->bodytext = bodytext;

		for (int n = 0; n < placeholders->size(); n++)
		{
			// create empty pieces of text
			placeHolderPointer.push_back(new OObjectPieceOfText(""));
		}
	}

	std::string OObject::getName()
	{
		return name;
	}

	std::string OObject::getKey()
	{
		return name + "__" + std::to_string(placeholders->size());
	}

	std::string OObject::getIdentifier()
	{
		if (placeholders == nullptr)
		{
			return name + "()";
		}
		else
		{
			return name + placeholders->toString();
		}
	}

	std::string OObject::getPlaceholders()
	{
		return placeholders->toString();
	}

	std::string OObject::getDocumentation()
	{
		return documentation;
	}

	std::string OObject::getBodytext()
	{
		return bodytext;
	}

	int OObject::getNrParam()
	{
		return placeholders->size();
	}

	bool OObject::identifierOK(const std::string &name, int nrp)
	{
		return (this->name == name && (nrp == getNrParam()));
	}

	void OObject::append(OrchestraReader *in)// throw(IOException)
	{
		readDocumentation(in);
		bodytext = bodytext + readBodytext(in);
		textPointers.clear();
		//initialiseTextPointers();
	}

	void OObject::insert(OrchestraReader *in) //throw(IOException)
	{
		readDocumentation(in);
		bodytext = readBodytext(in) + bodytext;
		textPointers.clear();
		//initialiseTextPointers();
	}

	void OObject::initialiseTextPointers()
	{
		// we should clear the list of textpointers in the C++ version
		// The java version simply creates a new one.
		// we should not only clear the list, but also delete the contents

		for (auto p : deleteableTextPointers) {
			delete p;
		}

		deleteableTextPointers.clear();

		textPointers.clear();

		std::string tmptext = StringHelper::trim(bodytext);

		std::string splitToken = "!"; // goes wrong when parameter names contain '!'. In text is no problem
		//textPointers = new std::vector<OObjectPieceOfText*>();

		// here we should add the code to select an alternative split character
		// if bodytext contains "!" then splitcharacter = "#"
		// else if body bodytext contains "#" then splitcharacter = ":"
		//if (StringHelper::contains(splitToken, bodytext)) { splitToken = "&"; }

		if (!StringHelper::contains("!", bodytext)) { splitToken = "!"; }		
		else if (!StringHelper::contains(":", bodytext)) { splitToken = ":"; }
		else if (!StringHelper::contains("#", bodytext)) { splitToken = "#"; }
		else if (!StringHelper::contains(";", bodytext)) { splitToken = ";"; }
		else { IO::showMessage("Could not find an unused splitcharacter in object text : " + tmptext); } 

		// 1) replace the original "<>" placeholder delimiters by single splittoken character 
		for (int n = 0; n < placeholders->size(); n++)
		{
			tmptext = StringHelper::replace(tmptext, "<" + placeholders->get(n) + ">", splitToken + placeholders->get(n) + splitToken);
			delete placeHolderPointer[n]; // can be zero
			placeHolderPointer[n] = new OObjectPieceOfText("?");
		}
			   
		// so we split the text in a set of tokens consisting of 
		// a sequence of text, parameter, text , parameter ....
//		std::vector<std::string> tokens = StringHelper::split(tmptext, '!');
		std::vector<std::string> tokens = StringHelper::split(tmptext, splitToken[0]); // please check!!!

		// the textpointers can point to a fixed piece of text, or to 
		// a piece of text managed by a placeholderpointer
		// the latter pieces of text are updated with new parameters 
		// when expanded
		for (auto token : tokens)
		{
			bool tokenIsParameter = false;

			// try to find the token in the list of placeholders
			for (int n = 0; n < placeholders->size(); n++)
			{
	    		if (token == placeholders->get(n))
				{
					tokenIsParameter = true;
					textPointers.push_back(placeHolderPointer[n]);
					break;
				}
			}

			if (!tokenIsParameter)
			{
				// we create a new piece of text here, that should be deleted upon destruction
				// 
				OObjectPieceOfText* Opt = new OObjectPieceOfText(token);
				textPointers.push_back(Opt);
                deleteableTextPointers.push_back(Opt);

			}

		}
	}

	std::string OObject::getSubstitutedBodytext(ParameterList *parameters)
	{
		
		if (textPointers.empty())
		{
			initialiseTextPointers();
		}

		// substitute the text of the parameters
		for (int n = 0; n < parameters->size(); n++)
		{
			placeHolderPointer[n]->text = parameters->get(n);
		}

		std::string totalText;

		// append all the pieces of text to a total substituted text
		for (auto v : textPointers){
			totalText +=  v->text;
		}
	
		return totalText;
	}
}
