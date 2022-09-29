#include <iostream>

#include "ExpressionString.h"
#include "stringhelper.h"

namespace orchestracpp
{

	ExpressionString::ExpressionString(const std::string &expressionIn)
	{

		int equalsPosition = (int)expressionIn.find("=");

        std::string tmp = expressionIn.substr(0, equalsPosition);
        tmp = StringHelper::replace(tmp, "{", " ");
        tmp = StringHelper::replace(tmp, "}", " ");
        resultVariableName = StringHelper::trim(tmp);
		
		tmp = expressionIn.substr(equalsPosition + 1, expressionIn.length() - (equalsPosition + 1));
        expression = StringHelper::trim(tmp);
    }

	std::string ExpressionString::getResultVariableName()
	{
		return resultVariableName;
	}

	std::string ExpressionString::getExpression()
	{
		return expression;
	}
}


