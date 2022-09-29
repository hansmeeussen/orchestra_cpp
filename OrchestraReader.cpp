#include "OrchestraReader.h"
#include "OrchestraException.h"
#include "FileBasket.h"
#include "Expander.h"

namespace orchestracpp
{

	OrchestraReader::OrchestraReader(Reader* in) {
		reader = in;
	}

	OrchestraReader::OrchestraReader(const std::string &string) {
		reader = new Reader(string);
	}

	std::string OrchestraReader::readWord(const std::string &separators) //throw(IOException)
	{
		readWhile(separators);
		return readUntil(separators);
	}

	std::string OrchestraReader::readWord()// throw(IOException)
	{
		return readWord(wordSeparators);
	}

	double OrchestraReader::readDouble()// throw(ReadException, IOException)
	{
		std::string tmp = StringHelper::replace(readWord(), ",", ".");
		try
		{
			return std::stod(tmp);
		}
		catch (const NumberFormatException &e)
		{
			throw new ReadException(e.what());
		}
	}

	double OrchestraReader::readDouble(const std::string &errmsg) //throw(ReadException, IOException)
	{
		try
		{
			return readDouble();
		}
		catch (const NumberFormatException &e)
		{
			std::string tmp{e.what()};
			throw ReadException("Number expected but \"" + tmp + "\" found.\n" + errmsg);
		}
	}

	int OrchestraReader::readInt() //throw(IOException)
	{
		return (std::stoi(readWord()));
	}

	std::string OrchestraReader::readLine()// throw(IOException)
	{
		std::string tempVar{'\n'};
		std::string tmp = (readUntil(tempVar));
		read(); // read the \n character
		return tmp;
	}

	std::string OrchestraReader::readUntil(const std::string &chars) //throw(IOException)
	{
		return read_(chars, _until);
	}


	std::string OrchestraReader::readWhile(const std::string &chars)// throw(IOException)
	{
		return read_(chars, _while);
	}


	std::string OrchestraReader::read_(const std::string &end_chars, bool until)// throw(IOException)
	{
		std::string tmpstring{""}; //create empty string

		do
		{
			int c = read();
			if (c == -1)
			{
				ready = true;
				break;
			}
			else if (contains(end_chars,c) == until)
			{
				reader->unread(c);
				break;
			}
			tmpstring += c;
		} while (true);

		return tmpstring;
	}

	int OrchestraReader::read()// throw(IOException)
	{

		int c = reader->read();
		if (!stripComment)
		{
			return c;
		}

		// nocomment == true, so strip comments from read text
		// this means that the complete line after // is discarded.
		while (c == '/')
		{ // found a single '/'
			c = reader->read(); // peek at the next character
			if (c == '/')
			{ // this was a double'//', indicating a comment line
				readLine(); // consume rest of the line
				return ('\n');
			}
			else
			{ // this was a single '/'
				reader->unread(c); // put back trailing character
				return ('/');
			}
		}

		return c;
	}


	void  OrchestraReader::unread(int c)// throw(IOException)
	{
		reader->unread(c);
	}

	void  OrchestraReader::unget()// throw(IOException)
	{
		reader->unget();
	}

	OrchestraReader *OrchestraReader::getOrchestraFileReader(FileID *fileID) //throw(IOException)
	{
		return getOrchestraFileReader(fileID->basket, fileID->name);
	}


	OrchestraReader *OrchestraReader::getOrchestraFileReader(FileBasket *fileBasket, const std::string &filename)// throw(IOException)
	{
		return new OrchestraReader(FileBasket::getFileReader(fileBasket, filename));
	}


	OrchestraReader* OrchestraReader::getExpandingFileReader(FileID * fileID)//throw (IOException)	
	{
		return getExpandingReader(getOrchestraFileReader(fileID), fileID->basket);
	}
	

	OrchestraReader* OrchestraReader::getExpandingReader(OrchestraReader* in, FileBasket *fileBasket)// throw  (IOException)
	{
		Expander exp{};
		StringWriter tmpwriter;
		exp.expand(in, &tmpwriter, fileBasket);
		return new OrchestraReader(tmpwriter.toString());
	}
	

	// check if string contains character
	bool OrchestraReader::contains(const std::string& str, const char& s) {
		if (str.find(s) != std::string::npos) {
			return true;
		}
		return false;
	}
}
