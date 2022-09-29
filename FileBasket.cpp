#include "FileBasket.h"
#include <iostream>
#include <istream>

namespace orchestracpp
{

	std::string FileBasket::workingDirectory = "";
	std::string FileBasket::fileSeparatorChar = "\\";
//	std::string FileBasket::fileSeparatorChar = "/";

	std::string FileBasket::getWorkingDirectory()
	{
		return workingDirectory;
	}

	std::string FileBasket::getWorkingDirectoryParent()
	{
		int end1 = (int)workingDirectory.rfind('/');
		int end2 = (int)workingDirectory.rfind('\\');
		int end = std::max(end1, end2);

		if (end <= 0)
		{
			return workingDirectory;
		}
		else
		{
			return workingDirectory.substr(0, end);
		}
	}

	std::string FileBasket::getFullFileName(const std::string &filename)
	{
		if (filename.find("/") != std::string::npos || filename.find("\\") != std::string::npos)
		{
			return filename;
		}

		if (workingDirectory == "")
		{
			return filename;
		}
		else
		{
			std::string tmp = (workingDirectory + fileSeparatorChar + filename);
			return tmp;
		}
	}

	bool FileBasket::fileExists(const FileBasket *fileBasket, const std::string &filename)
	{
		std::ifstream tmp1(getFullFileName(filename));
		if (tmp1.fail()) {
			return false;
		}
		else {
			return true;
		}
	}

	Reader *FileBasket::getFileReader(const FileBasket *fileBasket, const std::string &filename)// throw(IOException)
	{
		// First make sure that filename uses '/' as file separator
		std::string tmpfilename = StringHelper::replace(filename, "\\", "/");
		try
		{
			return getLocalFileReader(fileBasket, tmpfilename);
		}
		catch (const IOException &e)
		{
			throw IOException(e.what());
		}
	}

	Reader *FileBasket::getLocalFileReader(const FileBasket *fileBasket, const std::string &filename)// throw(IOException)
	{
		std::string tmp;
		 
		if (fileSeparatorChar == "/")
		{
			tmp = StringHelper::replace(filename, "\\", fileSeparatorChar);
		}
		else
		{ // fileseparatorchar is \ apple linux
			tmp = StringHelper::replace(filename, "/", fileSeparatorChar);
		}

		std::ifstream tmp1(getFullFileName(tmp));
		if (tmp1.fail()) {
		    throw IOException("Sorry, opening file: " + getFullFileName(tmp) + " was not successful! ");
	    }
		return new Reader(&tmp1);
	}

//	InputStream *FileBasket::getFileInputStream(FileBasket *fileBasket, const std::string &filename) throw(IOException)
//	{
//		FileInputStream tempVar(getFullFileName(filename));
//		return new BufferedInputStream(&tempVar);
//	}

	FileWriter* FileBasket::getFileWriter(const FileBasket *fileBasket, const std::string &filename) //throw(IOException)
	{
		//FileWriter tempVar(getFullFileName(filename));
		//return new BufferedWriter((&tempVar));
		return new FileWriter(filename);
	}

//	OutputStream *FileBasket::getFileOutputStream(FileBasket *fileBasket, const std::string &filename) throw(IOException)
//	{
//		FileOutputStream tempVar(getFullFileName(filename));
//		return new BufferedOutputStream(&tempVar);
//	}


    FileBasket::FileBasket(){}

}
