#pragma once

//----------------------------------------------------------------------------------------
// This helper class defines a number of static C++ string classes that mimic Java equivalents.
// Hans Meeussen, November 2019
//----------------------------------------------------------------------------------------
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>


namespace orchestracpp
{
class StringHelper
{
public:

	static bool contains(std::string substring, std::string characterset) {
		if (characterset.find(substring) != std::string::npos) {
			return true;
		}
		return false;
	}

	static bool contains(char substring, std::string characterset) {
		if (characterset.find(substring) != std::string::npos) {
			return true;
		}
		return false;
	}

	static bool equalsIgnoreCase(std::string string1, std::string string2) {
		if (string1 == string2) { 
			return true;
		}else if (toLower(string1) == toLower(string2)) {
			return true;
		}
		return false;
	}

	static std::string toLower(std::string source)
	{
		std::transform(source.begin(), source.end(), source.begin(), ::tolower);
		return source;
	}

	static std::string toUpper(std::string source)
	{
        std::transform(source.begin(), source.end(), source.begin(), ::toupper);
		return source;
	}

	static std::string trimStart(std::string source, const std::string &trimChars = " \t\n\r\v\f")
	{
		return source.erase(0, source.find_first_not_of(trimChars));
	}

	static std::string trimEnd(std::string source, const std::string &trimChars = " \t\n\r\v\f")
	{
		return source.erase(source.find_last_not_of(trimChars) + 1);
	}

	static std::string trim(std::string source, const std::string &trimChars = " \t\n\r\v\f")
	{
		return trimStart(trimEnd(source, trimChars), trimChars);
	}

	static std::string replace(std::string source, const std::string &find, const std::string &replace)
	{
		std::size_t pos = 0;
		while ((pos = source.find(find, pos)) != std::string::npos)
		{
			source.replace(pos, find.length(), replace);
			pos += replace.length();
		}
		return source;
	}

	static bool startsWith(const std::string &source, const std::string &value)
	{
		if (source.length() < value.length())
			return false;
		else
			return source.compare(0, value.length(), value) == 0;
	}

	static bool endsWith(const std::string &source, const std::string &value)
	{
		if (source.length() < value.length())
			return false;
		else
			return source.compare(source.length() - value.length(), value.length(), value) == 0;
	}

	static std::vector<std::string> split(const std::string &source, char delimiter)
	{
		std::vector<std::string> output;
		std::istringstream ss(source);
		std::string nextItem;

		while (std::getline(ss, nextItem, delimiter))
		{
			output.push_back(nextItem);
		}

		return output;
	}
	
	template<typename T>
	static std::string toString(const T &subject)
	{
		std::ostringstream ss;
		ss << subject;
		return ss.str();
	}

	static std::string doubleToString(const double &subject, const int precision)
	{
		std::ostringstream ss;
		ss << std::setprecision(precision)<<subject;
		return ss.str();
	}
	
};
}
