#include "IO.h"
#include "Expander.h"
#include "OObjectBasket.h"
#include "FileBasket.h"
#include "OObject.h"
#include "ParameterList.h"
#include "Parser.h"
#include "stringhelper.h"

using namespace std;
namespace orchestracpp
{
	std::string Expander::expandString(const std::string &inputstring) //throw(ReadException)
	{
		Expander expander; // so we create a tmp expander instance
		return expandString(inputstring, expander);
	}

	std::string Expander::expandString(const std::string& inputstring, Expander& expander) //throw(ReadException)
	{
		StringWriter sw;
		OrchestraReader sr(inputstring);
		expander.expand(&sr, &sw, nullptr);
		sw.toString();
		return sw.toString();
	}


	void Expander::expand(OrchestraReader *in, Writer *out, FileBasket *fileBasket) //throw(ReadException)
	{
		if (in == nullptr) {
			throw ReadException("expander input is null");
		}
		if (out == nullptr) {
			throw ReadException("expander output is null");
		}
		this->fileBasket = fileBasket;

		int c;
		try
		{
			while ((c = in->read()) != -1)
			{

				if (c == '@')
				{
					std::string word = in->readWord();
					std::string lcword = StringHelper::toLower(word);

					//Java switch (lcword)
					{
 
						if ((lcword == "include:") || (lcword == "scan:"))
						{
							ParameterList parameters(in,this);
							std::string filename;

							filename = parameters.get(0);
							filename = StringHelper::replace(filename, "\\", separatorChar);
							filename = StringHelper::replace(filename, "/", separatorChar);					

							if (filename.find(".docx") != std::string::npos)
							{
								std::cout << "sorry, .docx files not (yet) supported in the C++ expander version!" << std::endl;
							}
							else
							{
								OrchestraReader includeFile = *OrchestraReader::getOrchestraFileReader(fileBasket, filename);
								try
								{
									if (lcword == "scan:")
									{
										StringWriter sw;
										// we expand the input, but do not write text to the output
										IO::println("Scanning file: " + filename);
										expand(&includeFile, &sw, fileBasket);
										IO::println("Ready Scanning file: " + filename);

									}
									else {
										IO::println("Including file: " + filename);
									    expand(&includeFile, out, fileBasket);
								    }
								}
								catch (const std::runtime_error &e)
								{
									IO::showMessage("Could not open include file: " + filename);
									throw ReadException("Something went wrong trying to include: " + filename + " " + e.what());
								}
							}
						}
						else if ((lcword == "class:")|| (lcword == "def:"))
						{
							basket2.addObject(in);
						}
						else if ((lcword == "sclass:")||(lcword == "sharedclass:"))
						{ 
							IO::showMessage("Expander shared class is not yet inplemented in the C++ version");
						}
						else if (lcword == "append:")
						{
							basket2.append(in);
						}
						else if (lcword == "insert:")
						{
							basket2.insert(in);
						}
						else if (lcword == "delete:")
						{
							basket2.ddelete(in);
						}
						else if ((lcword == "expandedfile:")|| (lcword == "file:"))
						{
							// create file with expanded text between brackets
							std::string filename = OObject::readObjectName(in);
							OObject::readDocumentation(in); // only used to read to start of bodytext
                            std::string bodytext = OObject::readBodytext(in);
							FileWriter* tmpWriter = FileBasket::getFileWriter(fileBasket, filename); 

							if (lcword == "expandedfile:") {
								OrchestraReader tmpReader(bodytext);
								expand(&tmpReader, tmpWriter, fileBasket);
							}
							else {// (lcword == "file:") do not expand
								tmpWriter->write(bodytext);
							}
							tmpWriter->close();
                            delete tmpWriter;
						}
						else if (lcword == "warn:")
						{
							warnForUndefinedObjects = true;
						}
						else if (lcword == "nowarn:")
						{
							warnForUndefinedObjects = false;
						}
						else if (lcword == "evaluate:")
						{
							ParameterList parameters(in);
							if (parameters.size() == 1)
							{
								std::string expression = parameters.get(0);
								//StringWriter    *sw = new StringWriter();
								StringWriter sw;
								//OrchestraReader *sr = new OrchestraReader(expression);
								OrchestraReader sr(expression);
							
								// we expand the expressionstring before evaluating
								expand(&sr, &sw, fileBasket);
								try
								{
									out->write(StringHelper::toString(Parser::evaluate(sw.toString())));
								}
								catch (OrchestraException  e)
								{
									IO::showMessage("Something went wrong trying to evaluate expression: " + expression + " " + e.what());
								//	delete sw;
								//	delete sr;
								}
								//delete sw;
								//delete sr;
							}
						}
     					else if (lcword == "orchestraversion:")
						{
						    out->write("Orchestra c++ version!");
						}
						else
						{
						if (basket2.isObjectName(word))
							{
								//*********************************************************************************
								// 1) read the list of parameters from the inputstream
								ParameterList parameters(in, this);
								OObject *obj = basket2.getObject(word, parameters.size());
								if (obj != nullptr)
								{	
									// Todo, we need to expand the parameters first
									// this is now done with the new call to parameters
									OrchestraReader oreader(obj->getSubstitutedBodytext(&parameters));

									expand(&oreader, out, fileBasket);
								}
								else 
								{ // this was objectname with wrong number of parameters or deleted object so write original text

									//out->write(StringHelper::toString('@') + word + parameters.toString()+"\n");
									out->write(StringHelper::toString('@') + word + parameters.toString());
								}

								//*********************************************************************************
							}
//							else if (sharedBasket->isObjectName(word))
//							{
//								ParameterList parameters (in);
//
//								OObject *obj = sharedBasket->getObject(word, parameters.size());
//								if (obj != nullptr)
//								{
//									OrchestraReader oreader(obj->getSubstitutedBodytext(&parameters));
//									expand(&oreader, out, fileBasket);
//								}
//							}
							else
							{
								// word was not an object identifier
								if (this->warnForUndefinedObjects)
								{
									// check if nowWarnObjects contains lcword			
									if (std::find(nowarnObjects.begin(), nowarnObjects.end(), lcword) == nowarnObjects.end())
									{
										IO::println("WARNING: definition of @" + word + " not found!");
									}
								}
								out->write(StringHelper::toString('@') + word);
							}
						}
					} // end switch

				}
				else if (c == '/')
				{
					c = in->read();
					if (c == '/')
					{ // this is a comment line, do not expand
						out->write("//" + in->readLine() + "\n");
					}
					else
					{ // this was a single '/'
						out->write("/"); // so simply write
						in->unread(c); // and put back trailing character
					}
				}
				else
				{
				   out->write(c);
				}
			}
		}
		catch (const IOException &e)
		{
			throw ReadException(e.what());
		}
	}
}
