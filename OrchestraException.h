#pragma once

#include <string>
#include <stdexcept>
#include <exception>


namespace orchestracpp {

	class OrchestraException : public std::runtime_error {

	private:

	public:

		OrchestraException(const std::string& msg) :
			std::runtime_error(msg)
		{
		}
		~OrchestraException() {}

	};

	class ParserException : public OrchestraException {

	public:
		ParserException(const std::string& msg) :
			OrchestraException(msg)
		{
		}
	};


	class ReadException : virtual public OrchestraException {
	public:
		ReadException(const std::string& msg) :
			OrchestraException(msg)
		{
		}

	};


	class IOException : virtual public OrchestraException {
	public:
		IOException(const std::string& msg) :
			OrchestraException(msg)
		{
		}
	};


	class NumberFormatException : virtual public OrchestraException {
	public:
		NumberFormatException(const std::string& msg) :
			OrchestraException(msg)
		{
		}

	};

	class ExitException : virtual public OrchestraException {
	public:
		ExitException(const std::string& msg) :
			OrchestraException(msg)
		{
		}

	};

}
