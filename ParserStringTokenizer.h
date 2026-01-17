#pragma once

#include <string>
#include "StringTokenizer.h"
#include "OrchestraException.h"

namespace orchestracpp { class Var; }
namespace orchestracpp { class VarGroup; }

namespace orchestracpp
{
	
	/**
	 * This string tokenizer splits up a string in tokens.
	 * It does not remove the delimiter characters
	 */
	class ParserStringTokenizer final
	{
	private:
		std::string expression; // the expression string
		StringTokenizer *tokenizer = nullptr;
		std::string currentToken; // the current nextToken, can be null;

	public:
        
        ParserStringTokenizer();
        
		~ParserStringTokenizer()
		{
			delete tokenizer;
		}

		ParserStringTokenizer(const std::string &expression);

	    std::string getCurrentToken();

		std::string nextToken() /*throw(ParserException)*/;

		void consume();

		bool match(const std::string &s)/* throw(ParserException)*/;

	private:
		void matchAndConsume(const std::string &s, const std::string &message)/* throw(ParserException)*/;

	public:
		void matchAndConsume(const std::string &s)/* throw(ParserException)*/;

		bool equals(const std::string &s)/* throw(ParserException)*/;

		bool hasMoreTokens();

		bool isNumber();

		bool isaNumber(const std::string);

        Var *isVariable(VarGroup *variables);

		std::string getExpression();
		

	};




}
