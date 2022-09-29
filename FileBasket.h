#pragma once

#include <string>
#include "OrchestraException.h"
#include "OrchestraReader.h"

namespace orchestracpp
{

	class FileBasket
	{

	public:

		static std::string fileSeparatorChar;

		static std::string workingDirectory; // this is the path to the working directory

		static std::string getWorkingDirectory();

		static std::string getWorkingDirectoryParent();

		static std::string getFullFileName(const std::string &filename);

		static bool fileExists(const FileBasket *fileBasket, const std::string &filename);
		
		static Reader *getFileReader(const FileBasket *fileBasket, const std::string &filename) /*throw(IOException)*/;

		static Reader *getLocalFileReader(const FileBasket *fileBasket, const std::string &filename) /*throw(IOException)*/;

		//static InputStream *getFileInputStream(FileBasket *fileBasket, const std::string &filename) throw(IOException);

		static FileWriter *getFileWriter(const FileBasket *fileBasket, const std::string &filename) /*throw(IOException)*/;

		//static OutputStream *getFileOutputStream(FileBasket *fileBasket, const std::string &filename) throw(IOException);

		FileBasket();
	};
}