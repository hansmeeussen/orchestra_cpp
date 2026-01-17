#include "OObjectBasket.h"
#include "OObject.h"
#include "OrchestraReader.h"
#include "ParameterList.h"

namespace orchestracpp
{

	OObjectBasket::~OObjectBasket()
	{
		for (auto pair : newObjects) {
			delete pair.second;
		}

		//for (std::unordered_map<std::string, OObject*>::iterator itr = newObjects.begin(); itr != newObjects.end(); itr++)
		//{
		//	delete (itr->second);
		//}
	}

	OObject *OObjectBasket::addObject(OrchestraReader *in)// throw(IOException)
	{
//		std::cout << "adding and constructing object:" << std::endl;

		std::string n     = OObject::readObjectName(in);
		ParameterList* pl = OObject::readParameterList(in);
		std::string d     = OObject::readDocumentation(in);
		std::string b     = OObject::readBodytext(in);


		// eat the end of line character after the object definition
		int c = in->read();
		if (c != '\n') {
			in->unread(c);
		}

//		OObject* obj = new OObject(OObject::readObjectName(in), OObject::readParameterList(in), OObject::readDocumentation(in), OObject::readBodytext(in));

		OObject* obj = new OObject(n, pl, d, b);


//		std::cout << "adding object: " << obj->getName() << std::endl << obj->getPlaceholders() << std::endl << obj->getBodytext() << std::endl;

		// we should remove or overwrite existing objects?? 


	    // we should delete the overwritten object if this exists

		newObjects.erase(obj->getKey());

		newObjects.emplace(obj->getKey(), obj);
		//newObjects.insert(obj->getKey(), obj);
		objectNames.emplace(obj->getName());

		return obj;
	}

	OObject *OObjectBasket::append(OrchestraReader *in)// throw(IOException)
	{
		OObject *obj = getObject(in);
		if (obj != nullptr)
		{
			obj->append(in);
		}
		else
		{
			//should create OObject here ?           
		}
		return obj;
	}

	OObject *OObjectBasket::insert(OrchestraReader *in) //throw(IOException)
	{
		OObject *obj = getObject(in);
		if (obj != nullptr)
		{
			obj->insert(in);
		}
		else
		{
			//should create OObject here ?
		}
		return obj;
	}

	OObject *OObjectBasket::ddelete(OrchestraReader *in)// throw(IOException)
	{
		OObject *obj = getObject(in);
		newObjects.erase(obj->getKey());
		// we do not remove the name from the objectnames, because there may
		// be more objects with the same name (but different number of parms)
		return obj;
	}

	bool OObjectBasket::isObjectName(const std::string &name)
	{
		return (objectNames.find(name) != objectNames.end());
		//return objectNames.contains(name);
	}

	OObject *OObjectBasket::getObject(const std::string &name, int nrParam)
	{
		return static_cast<OObject*>(newObjects[name + "__" + std::to_string(nrParam)]);
	}

	OObject *OObjectBasket::getObject(OrchestraReader *in) //throw(IOException)
	{
		std::string name = OObject::readObjectName(in);
		ParameterList tempVar(in);
		return (getObject(name, (&tempVar)->size()));
	}
}
