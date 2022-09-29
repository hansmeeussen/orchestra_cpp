#include "ParameterList.h"
#include "stringhelper.h"
#include "OrchestraReader.h"
#include "Parser.h"
#include "IO.h"
#include <iostream>
#include <fstream>
#include <string>
#include "OObject.h"

using namespace std;

namespace orchestracpp
{

	ParameterList::ParameterList(const std::string &s) : ParameterList( new OrchestraReader (s))
	{
    }

	ParameterList::ParameterList(OrchestraReader* in) : ParameterList(in, nullptr)
	{
	}

	ParameterList::ParameterList(OrchestraReader * in, Expander * expander)
	{
		strings = std::vector<std::string>();
		int c;
		std::string separators;  //characters that separate string
		std::string terminators; //character(s) that terminate string list
		std::string septerm;     //character(s) that terminate/separate string list

		// consume space and tab characters
		in->readWhile(spaceOrTab);
		c = in->read();

		// set string separators and list terminators according to format
		if (c == '(')
		{ // format 1: (name1, name2, name3, "een tekstje")
			separators = format1Separators;
			terminators = format1Terminators;
			septerm = format1SepAndTerm;
		}
		else
		{ // format 2: name1 name2 name3 "een tekstje" eol      //comments
			separators = format2Separators;
			terminators = format2Terminators;
			septerm = format2SepAndTerm;
			in->unread(c);
		}

		//while (true)	
		while (!in->ready)
		{
			//read string separators (if any)
			in->readWhile(separators);

			//read word
			std::string tmpstring = "";

			c = in->read();

			if (c == '"')
			{
				bool old_StripComment = in->stripComment;
				in->stripComment = false;
				tmpstring += (in->readUntil("\""));
				in->stripComment = old_StripComment;
				in->read(); // remove trailing "

			}


			// this is new (5/10/2020, for expanding object references and expand statements

			else if ((c == '@') && (expander != nullptr))

			{// this parameterlist contains an object reference or evaluate statement
				std::string  name = OObject::readObjectName(in);


				if (expander->basket2.isObjectName(name)) { // need to put basket in expander
					ParameterList parameters (in);
					// now expand the name and parameterlist
					std::string input = "@" + name + parameters.toString();

					try {
						tmpstring += Expander::expandString(input,*expander);  
					}
					catch (ReadException ioe) {
						IO::showMessage("Something went wrong trying to expand in a  parameterlist: " + input + " " + ioe.what());
					}
				}
				else if (StringHelper::equalsIgnoreCase(name,"evaluate:")) {
					ParameterList parameters(in);
				    try {
				     	 tmpstring+= StringHelper::toString( Parser::evaluate(Expander::expandString(parameters.get(0), *expander)));
				    }
				    catch (ParserException pe) {
						IO::showMessage("Something went wrong trying to expand in a  parameterlist: " + parameters.toString() + " " + pe.what());
				    }
				
			    }

			    else
			    {// it is not an objectname or evaluate:
			         tmpstring+=("@");
			         tmpstring+=(name);
			    }

				
		    }

			else
			{ //check whether comment and return if so (leave // in stream)
				if (c == '/')
				{
					c = in->read();
					if (c == '/')
					{
						in->unread('/');
						in->unread('/');
						return;
					}
					else
					{
						in->unread(c);
					}
				}
				else {
					in->unread(c);
					tmpstring += in->readUntil(septerm);//includes terminators
				}
			}

			if (tmpstring.size() > 0)
			{
				strings.push_back(tmpstring);
			}

			//determine whether we have to stop
			c = in->read();
			if (StringHelper::contains(c, terminators))
			{
				if (c != ')')
				{
					// this is a terminator, but not a ')' so put back and return
					in->unread(c);
				}
				return;
			}
			else
			{
				in->unread(c);
			}
		}
	
    }


	std::string ParameterList::toString() {
		StringWriter tmpuit;


		tmpuit.write("(");

		for (int n = 0; n < strings.size(); n++) {

			// here we should add enclosing "" if parameter contains terminators/separators
			if (StringHelper::contains("(", strings[n]) || 
				StringHelper::contains("@", strings[n]) || 
				StringHelper::contains(" ", strings[n]) || 
				StringHelper::contains(",", strings[n])) {
				tmpuit.write("\""); 
				tmpuit.write(strings[n]); 
				tmpuit.write("\"");
			}
			else {
		    	tmpuit.write(strings[n]);
		    }
			if (n<strings.size()-1){ // no comma after last placeholder
				tmpuit.write(", ");
			}
		}

		tmpuit.write(")");

		return tmpuit.toString();
	}

	int ParameterList::size() {
		return (int)strings.size();
	}


	std::string ParameterList::get(int i) {
		return strings[i];
	}

	void ParameterList::set(int i, const std::string &value) {
		strings[i] = value;
	}


	double ParameterList::getDouble(int i) {
		return std::stod(strings[i]);
	}


	int ParameterList::index(const std::string &s){

		for (int n = 0; n < this->size(); n++)	{
			// why do we use ignorecase here? test java version
			if (StringHelper::equalsIgnoreCase(s, get(n))) {
				return n;
			}
		}
		return -1;
	}

	bool ParameterList::contains(const std::string &n) {
		return index(n) >= 0;
	}
}
