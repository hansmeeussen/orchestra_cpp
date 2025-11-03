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
	class ParserStringTokenizer
	{
	private:
		std::string expression; // the expression string
		StringTokenizer *tokenizer = nullptr;
		std::string currentToken; // the current nextToken, can be null;

	public:
        
        ParserStringTokenizer();
        
		virtual ~ParserStringTokenizer()
		{
			delete tokenizer;
		}

		ParserStringTokenizer(const std::string &expression);

		virtual std::string getCurrentToken();

		virtual std::string nextToken() /*throw(ParserException)*/;

		virtual void consume();

		virtual bool match(const std::string &s)/* throw(ParserException)*/;

	private:
		void matchAndConsume(const std::string &s, const std::string &message)/* throw(ParserException)*/;

	public:
		virtual void matchAndConsume(const std::string &s)/* throw(ParserException)*/;

		virtual bool equals(const std::string &s)/* throw(ParserException)*/;

		virtual bool hasMoreTokens();

		virtual bool isNumber();

		virtual bool isaNumber(const std::string);

        virtual Var *isVariable(VarGroup *variables);

		virtual std::string getExpression();
		

	};




}
