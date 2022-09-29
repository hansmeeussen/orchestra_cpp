#pragma once

#include <string>

namespace orchestracpp
{

	 /**
	 * This class represents an expression string that is later added to the ExpressionGraph
	 */
	class ExpressionString final
	{
	public:
		std::string resultVariableName="";
        
		std::string expression="";

	private: ExpressionString() {}

	public:

		ExpressionString(const std::string &expression);

		std::string getResultVariableName();

		std::string getExpression();
	};

}
