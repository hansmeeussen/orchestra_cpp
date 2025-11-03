#include "ParserStringTokenizer.h"
#include "stringhelper.h"
#include "VarGroup.h"
#include "IO.h"

namespace orchestracpp
{

    ParserStringTokenizer::ParserStringTokenizer(const std::string &expression)
	{
		this->expression = expression;
		tokenizer = new StringTokenizer(this->expression," \t*/+-()^!<>&|,={}",true);
	}
	
	std::string ParserStringTokenizer::getCurrentToken()
	{
		return currentToken;
	}

	std::string ParserStringTokenizer::nextToken()// throw(ParserException)
	{
		if (currentToken == "")
		{ // eat spaces
			while (hasMoreTokens())
			{
                currentToken = tokenizer->nextToken();
				if (currentToken == " " || currentToken == "\t")
				{
					 // do nothing
				}
				else
				{
					break;
				}
			}
		}

		if (!hasMoreTokens() && (currentToken == ""))
		{
			return "";
		}

		// return complete nextToken within {} without parsing
		if (currentToken == "{")
		{
            std::string tmpToken;
			while (true)
			{
				std::string tmp = tokenizer->nextToken();
				if (tmp == "}")
				{
					currentToken = tmpToken; 
					break;
				}
				else
				{
					tmpToken = tmpToken + tmp;
				}

				if (!hasMoreTokens())
				{
					//delete tmpToken;
					throw OrchestraException("No matching } in expression: {" + currentToken);
				}
			}
		}

		// If nextToken starts with a digit and ends with 'e' then this nextToken could be the first part of a number
		// that has a negative exponent e.g 3.07e-7
		if ((currentToken[0] >= '0') && (currentToken[0] <= '9'))
		{
			if ((currentToken[currentToken.length() - 1] == 'e') || (currentToken[currentToken.length() - 1] == 'E'))
			{
			//	try
			//	{ // make sure that this nextToken represents a number
					//static_cast<Double>(currentToken.substr(0,(currentToken.length() - 1)));
					// this was a number ending with a single "e"

					// try to read string (minus last character) into double
					//double test = std::stod(currentToken.substr(0, (currentToken.length() - 1)));
					
				if (isaNumber(currentToken.substr(0, (currentToken.length() - 1)))) {
					currentToken = currentToken + tokenizer->nextToken();
					currentToken = currentToken + tokenizer->nextToken();
				}
				else {
					//This was not a number, simply return nextToken
				}

/*
					if (((std::isnan(test)) || std::isinf(test))) {

					}
					else {

					// this worked, so was a number

					    currentToken = currentToken + tokenizer->nextToken();
					    currentToken = currentToken + tokenizer->nextToken();
				    }
				}
                catch (const std::invalid_argument& ia)
				{
					//This was not a number, simply return nextToken
				}
				*/
				
			}
		}

		return currentToken;
	}

	void ParserStringTokenizer::consume()
	{
		currentToken = "";
	}


	bool ParserStringTokenizer::match(const std::string &s)// throw(ParserException)
	{
		std::string tmptoken = nextToken();
		if (tmptoken == "")
		{
			return false;
		}
		return s.find(tmptoken) != std::string::npos;
	}

	void ParserStringTokenizer::matchAndConsume(const std::string &s, const std::string &message)// throw(ParserException)
	{
		if (match(s))
		{
			consume();
		}
		else
		{
			throw OrchestraException(message);
		}
	}

	void ParserStringTokenizer::matchAndConsume(const std::string &s)// throw(ParserException)
	{
		matchAndConsume(s, "\"" + s + "\" expected!!");
	}

	bool ParserStringTokenizer::equals(const std::string &s) //throw(ParserException)
	{
		std::string tmptoken = nextToken();
		if (tmptoken == "")
		{
			return false;
		}
		return (StringHelper::toLower(s)==(StringHelper::toLower(tmptoken)));
	}

	bool ParserStringTokenizer::hasMoreTokens()
	{
		return tokenizer->hasMoreTokens();
	}

	bool ParserStringTokenizer::isNumber()
	{
		if (currentToken == "")
		{
			return false;
		}

		/*
		try
		{
			double test = std::stod(currentToken); 

            // the std::stod function returns nan when the input string is a NANsequence
			// that is "NAN..............
			// This means that nantokite, NaNO3 are not recognized as a variable, but as a number (nan)
			// NANsequence
			// Therefore we check the resulting number for nan and infinity			
			if (std::isnan(test)) {
				return false;
			}
			if (std::isinf(test)) {
				return false;
			}
		}
        catch (const std::invalid_argument& ia) {
            return false;   
        }
		*/


		// This version correctly identifies stings that start with a number as a string
		// 2KO3-
		// nantokite
		// NaNO3-

//		long double ld;
//		if ((std::istringstream(currentToken) >> ld >> std::ws).eof()) {
//
//			//IO::showMessage("This is a number " + currentToken);
//
//			return true;
//		}
//		else {
//			return false;
//		}

		return isaNumber(currentToken);
	}

	bool ParserStringTokenizer::isaNumber(const std::string s) {
		long double ld;
		return((std::istringstream(s) >> ld >> std::ws).eof());

		/*
		long double ld;
		if ((std::istringstream(s) >> ld >> std::ws).eof()) {
			return true;
		}
		else {
			return false;
		}
		*/
		// Return true on valid
	//	bool valid_string_to_double(const char* s) {
	//		char* end;
	//		strtod(s, &end);
	//		return s != end;
		
	}

	Var *ParserStringTokenizer::isVariable(VarGroup *variables)
	{
		if ((currentToken == "") || (variables == nullptr))
		{
			return nullptr;
		}
		return variables->get(currentToken);
	}

	std::string ParserStringTokenizer::getExpression()
	{
		return expression;
	}
	
}
