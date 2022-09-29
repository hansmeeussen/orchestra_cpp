#pragma once

#include <string>
#include <iostream>
#include <istream>
#include <fstream>
#include <sstream>
#include "OrchestraException.h"
#include "stringhelper.h"
#include "FileID.h"

namespace orchestracpp
{
    class Writer { // the base class

	public:
		virtual void write(const std::string&) = 0;
		virtual void write(const char) = 0;
		virtual std::string toString() = 0;
	};


	// are we using this one already? should be tested
	// java works a bit different from c++
	// in  jave we use an internal string, which is returned 
	// in the tostring method
	class StringWriter : public Writer {
		std::ostringstream out;

	public:

		StringWriter() {
			out = std::ostringstream();
		}

		void write(const std::string& s) {
			out << s;
		}

		void write(const char s) {
			out << s;
		}

		std::string toString() {
			return out.str();
		}

	};



	class FileWriter : public Writer {
		std::ofstream out;
	public:

		FileWriter(const std::string &filename) {
			out = std::ofstream(filename);
		}

		void write(const std::string& s) {
			out << s;
		}

		void write(const char s) {
			out << s;
		}

		std::string toString() {
			return "";
		}

		void close() {
			out.close();
		}

	};




	// the Reader baseclass 
	// we should be able to construct a reader from file or string
	// so we do not need as separate stringreader and filereader

	class Reader {
	//	static const istream in;
		std::istringstream in;

	public:
		Reader() {}

		// reader from string
		Reader(const std::string &text) {
			in = std::istringstream(text);
		}

		// reader from asci file
		Reader(std::ifstream *inputfile) {
			std::stringstream buffer;
			buffer << inputfile->rdbuf();
			in = std::istringstream(buffer.str());
			inputfile->close();
		}

		virtual int read() {
			return in.get();
		}

		virtual void unread(int c) {
			in.putback(c);
		}

		virtual void unget() {
			in.unget();
		}

	};

	class FileReader :public Reader {
		std::ifstream* ifs = nullptr;
		FileReader(const std::string &filename) {
			ifs = new std::ifstream(filename);
			Reader(ifs);
		}

		~FileReader() {
			delete ifs;
		}
	};

	/**
	 * The OrchestraReader class extends a PushbackReader and enables reading space or tab
	 * separated words (like in C++) and lines from a stream.
	 *
	 * @author: Hans Meeussen 11/10/98 (7654321)
	 */
	class OrchestraReader// : public Reader
		// we have changed this so that an OrchestraReader contains a Reader rather than being a reader
		// all calls to Reader methods are passed through to the reader
	{

	public:
		std::string wordSeparators{ ' ', '\t', '\n', '\r', '(' };
		std::string lineSeparators{ '\r', '\n' };
		std::string spaceOrTab{ '\t', ' ' };

		static constexpr bool _while = false;
		static constexpr bool _until = true;

		Reader* reader = nullptr;

		bool ready = false;
		// by default comment stripping is on
		bool stripComment = true; // set to false when comment needs to be read

		OrchestraReader(Reader*);

		OrchestraReader(const std::string&);

		~OrchestraReader() {
			delete reader;
		}

		virtual std::string readWord(const std::string &separators) /*throw(IOException)*/;

		// reads a space, tab, newline, return, and ( separated word
		virtual std::string readWord() /*throw(IOException)*/;

		virtual double readDouble();// throw(ReadException, IOException);

		virtual double readDouble(const std::string &errmsg)/* throw(ReadException, IOException)*/;

		virtual int readInt() /*throw(IOException)*/;

		virtual std::string readLine()/* throw(IOException)*/;

		virtual std::string readUntil(const std::string &chars)/* throw(IOException)*/;

		// single character
		//virtual std::string *readUntil(char c) throw(IOException);

		virtual std::string readWhile(const std::string &chars) /*throw(IOException)*/;

		// single character
		//virtual std::string *readWhile(char c) throw(IOException);

		// reads string from reader until end_character(s)
	private:
		std::string read_(const std::string &end_chars, bool until) /*throw(IOException)*/;

		// this overloaded version of read removes comments from reader
		// if nocomment is true
	public:
		virtual int read()/* throw(IOException)*/;

		virtual void unread(int c);// throw(IOException);

		virtual void unget();// throw(IOException);

		static OrchestraReader *getOrchestraFileReader(FileID *fileID)/* throw(IOException)*/;

		/**
		 * get a file reader from a basket by name
		 */
		static OrchestraReader *getOrchestraFileReader(FileBasket *fileBasket, const std::string &filename)/* throw(IOException)*/;
		
		static OrchestraReader* getExpandingFileReader(FileID * fileID)/*throw (IOException)*/;
	
		static OrchestraReader* getExpandingReader(OrchestraReader* in, FileBasket *fileBasket) /*throw  (IOException)*/;
	
		bool contains(const std::string&, const char&);

	
	};

}
