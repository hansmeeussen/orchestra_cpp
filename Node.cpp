#include "Node.h"
#include "NodeType.h"
#include "IO.h"


namespace orchestracpp
{

	Node::Node(NodeType *nodeType)
	{
		this->nodeType = nodeType;

		if (!nodeType->isInitialised)
		{ //this is the first node created
			nodeType->initialise();
		}

		values = new double[nodeType->nrVars];

		for (int n = 0; n < nodeType->nrVars; n++)
		{
			values[n] = nodeType->defaultValues[n];
		}
	}

	double Node::getvalue(int index)
	{
		if (index < 0)
		{
			return 0;
		}
		if (index < nodeType->nrVars)
		{ // normal variable
			return values[index];
		}
		else
		{ // static variable stored in nodeType
			return nodeType->staticValues[index - nodeType->nrVars];
		}
	}

	void Node::setValue(int index, double value)
	{
		if (index < 0)
		{
			return;
		}
		if (index < nodeType->nrVars)
		{ // normal variable
			values[index] = value;
		}
		else
		{ // static variable
			nodeType->staticValues[index - nodeType->nrVars] = value;
		}
	}

	void Node::clone(Node *original)
	{
			if (this->nodeType == original->nodeType)
			{
				// skip first value (NodeID)
				// this can be done quicker in c++
				
				//values = original->values;

				for (int n = 1; n < nodeType->nrVars; n++)
				{
					values[n] = original->values[n];
				}
			}
			else
			{
				IO::println("Copying nodes of a different type is not implemented in C++ yet!! ");
			// This can be done quicker!
				// and also has a problem with the first index NodeID!! Is never used 
//				for (auto name : this->nodeType->names)
//				{
//					int originalindex = original->nodeType->index(name);
//					if (originalindex >= 0)
//					{ // variable exists in original node
//						this->setValue(this->nodeType->index(name), original->getvalue(originalindex));
//					}
//				}
			}
       
	}

	void Node::nodeBetween(Node *left, Node *right, double f)
	{

		for (int n = 1; n < nodeType->nrVars; n++){ // skip first variable (Node_ID)
			values[n] = ((1.0 - f) * left->values[n] + (f)* right->values[n]);
	    }

	}

	Node *Node::clone()
	{
		Node *tmpNode = new Node(nodeType);
		tmpNode->clone(this);
		return tmpNode;
	}

	bool Node::containsNaN()
	{
		for (int n = 1; n < nodeType->nrVars; n++) {
			
			if ((NAN == values[n]) || std::isnan(values[n]) || std::isinf(values[n]))
			{
				return true;
			}
		}
		return false;
	}
}
