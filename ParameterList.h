#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include "OrchestraException.h"
#include "stringhelper.h"
#include "OrchestraReader.h"
#include "Expander.h"

namespace orchestracpp
{
	/**
	 * ParameterList reads a set of strings from an input stream and inserts them
	 * into a parameterlist. Represents list of parameters/ placeholders in object
	 * definitions It recognises two different formats
	 * 1) strings separated by comma's and started and terminated by brackets 
	 * 2) strings separated by spaces tabs and list terminated by eol
	 */
	class ParameterList final
	{

	private:
		std::vector<std::string> strings;

	public:

		std::string format1Separators  { ',', ' ', '\t', '(', '\n', '\r' };
		std::string format1Terminators { ')' };
		std::string format1SepAndTerm  { ',' , ' ', '\t', '(', '\n', '\r', ')' };

		std::string format2Separators  { ' ' , '\t', ',' };
		std::string format2Terminators { '\n', '\r', '}', '{' };
		std::string format2SepAndTerm  { ' ' , '\t',',', '\n', '\r', '}', '{' };

		std::string spaceOrTab         { '\t', ' ' };

		ParameterList(const std::string &s);

		ParameterList(OrchestraReader *in);

		ParameterList(OrchestraReader* in, Expander * expander);


		std::string toString();

		int size();

		std::string get(int i);

		void set(int i, const std::string &value);

		double getDouble(int i);

		int index(const std::string &s);

		bool contains(const std::string &n);
	};

}
