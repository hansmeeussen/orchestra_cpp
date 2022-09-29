#pragma once

#include <string>
#include <iostream>
#include <cmath>
#include "stringhelper.h"
#include "OrchestraException.h"

namespace orchestracpp
{

	/**
	 * The IO class contains a set of static methods that take care of ALL ORCHESTRA
	 * IO actions. This is convenient and makes it possible to e.g. redirect or
	 * suppress all output.
	 *
	 * @author Hans Meeussen
	 */
	class IO
	{

	public:
		//static JFrame *frame;
		static std::string totaltext;
		//static JPanel *panel;
		//static JTextArea *tArea;

		static bool verbose;
		static bool modal;
		static bool showprogress;
		static bool testing; // indicates whether test output is generated
		//static const std::string separatorChar{ "//" }; // should get a value

//		static void startLogging();

		static void print(const std::string &text);

		// only print when testing is true
//		static void printTest(const std::string &text);

		static void println(const std::string &text);

//		static void printlnTest(const std::string &text);

		static void showMessage(const std::string &message);

//		static void showMessageTest(const std::string &message);

		static void showMessage(std::runtime_error e);

//		static void showMessageTest(std::runtime_error e);

		static double parseDouble(const std::string &s);

		/**
		 * A string formatting routine for double values
		 */
		static std::string format(double x, int fieldwidth, int nr_decimals);

		static double checkNAN(double x, const std::string &message) /*throw(OrchestraException)*/;

		static std::string format(double x, int fieldwidth, int nrDecimals, char dotcom);

//		static std::string format2(double x, int fieldwidth, int nr_decimals, char dotcom);

		static std::string format(const std::string &s, int fieldwith);

	};

}
