#include "IO.h"
#include "OrchestraException.h"

namespace orchestracpp
{
std::string IO::totaltext;
bool IO::verbose = true;
bool IO::modal = true;
bool IO::showprogress = false;
bool IO::testing = false;


	void IO::print(const std::string &text)
	{
		std::cout << text;
		return;		
	}


	void IO::println(const std::string &text)
	{
		print(text + "\n");
	}


	void IO::showMessage(const std::string &message)
	{
		println(message);
	}


	void IO::showMessage(std::runtime_error e)
	{
		showMessage(e.what());
	}


	double IO::parseDouble(const std::string &s)
	{
		std::string tmp = StringHelper::replace(s, ",", ".");
		return std::stod(tmp);
	}

	std::string IO::format(double x, int fieldwidth, int nr_decimals)
	{
		return format(x, fieldwidth, nr_decimals, '.');
	}

	double IO::checkNAN(double x, const std::string &message)// throw(OrchestraException)
	{
		if (/*(NAN == x) || */ std::isnan(x) || std::isinf(x))
		{
			throw OrchestraException(message);
		}
		return x;
	}

	
	std::string IO::format(double x, int fieldwidth, int nrDecimals, char dotcom)
	{
		std::string tmpString("                                                                                                     ");
		long long expon;
		long long fraction;
		bool negative = false;
		long long nd = (long long)std::pow(10, nrDecimals);
		long long ndx10 = static_cast<long long>(nd) * 10;
		double nanindicator = 123456789;
		if (NAN == x)
		{
			x = nanindicator;
		}
		else if (std::isnan(x))
		{
			x = nanindicator;
		}
		else if (std::isinf(x))
		{
			x = nanindicator;
		}

		if (x < 0)
		{
			negative = true;
			x = -x;
		}

		if (x == 0)
		{
			//tmpstring.insert(1, "0" + StringHelper::toString(dotcom) + "0");
			tmpString.replace(1, 3, "0" + StringHelper::toString(dotcom) + "0");
		}
		
		else
		{
			expon = static_cast<long long>(std::floor(std::log10(x)));

			// fraction is an integer representing the 3.14343545
			fraction = static_cast<long long>(static_cast<long long>(std::round((nd * x) / std::pow(10, static_cast<double>(expon)))));
			if (fraction == ndx10)
			{
				fraction = fraction / 10;
				expon = expon + 1;
			}

			switch (expon)
			{
			case 0:
				tmpString.insert(0, StringHelper::toString(fraction));
				tmpString.insert(1, StringHelper::toString(dotcom));
				break;
			case 1:
				tmpString.insert(0, StringHelper::toString(fraction));
				tmpString.insert(2, StringHelper::toString(dotcom));
				break;
			default:
				tmpString.insert(0, StringHelper::toString(fraction) + "e" + StringHelper::toString(expon));
				tmpString.insert(1, StringHelper::toString(dotcom));
				break;
			}

			if (negative)
			{
				tmpString.insert(0, "-");
			}
			else
			{
				tmpString.insert(0, " ");
			}
		}
		
		//tmpString.setLength(fieldwidth);
		tmpString = tmpString.substr(0, fieldwidth);
		return tmpString;
	}

	
/*
	std::string IO::format2(double x, int fieldwidth, int nr_decimals, char dotcom)
	{
		std::string tmpstring;

		// first we create a number 10 to the power of nr_decimals
		long nd = std::pow(10, nr_decimals);

		//StringBuilder *tmp;
		try
		{
			std::string tmp = std::round(nd * x);

			tmp.insert(std::max(0, (tmp.length() - static_cast<int>(nr_decimals))), dotcom);

			tmpstring = "";
			for (int n = 1; n < fieldwidth - tmp.length(); n++)
			{
				tmpstring+=(' ');
			}
			tmpstring += tmp;
		}
		catch (const std::runtime_error &e)
		{
			IO::showMessage(e.what() + " " + std::to_string(x));
		}

		return tmpstring;
	}
	*/


	std::string IO::format(const std::string &s, int fieldwidth)
	{
		std::string  tmpString("                                                                                                     ");
		tmpString.insert(1, s);
		tmpString = tmpString.substr(0, fieldwidth);
		return tmpString;
	}
	
}
