#include "NodeIOObject.h"
#include "NodeIOPair.h"
#include "NodeType.h"
#include "Var.h"
#include "IO.h"
#include "VarGroup.h"
#include "Node.h"

namespace orchestracpp
{

	NodeIOObject::NodeIOObject(const std::string &prefix, VarGroup *variables, NodeType *nodeType)
	{
		int index = 0;
		std::vector<Var*> usedVars; // contains already used variables

		for (auto globalName : nodeType->names)
		{
			Var *local = variables->get(prefix + globalName);

			if (local != nullptr)
			{ // We found a variable
				if (std::find(usedVars.begin(), usedVars.end(), local) == usedVars.end())
				{ // is not yet used
					usedVars.push_back(local);
					nodeIOPairs.push_back(new NodeIOPair (local, index));
				}
			}
			index++;
		}

	}

	void NodeIOObject::copyToLocal(Node *globalNode)
	{
		for (auto p : nodeIOPairs) {
			p->copyFromNode(globalNode);
		}
	}

	void NodeIOObject::copyToGlobal(Node *globalNode)
	{
		for (auto p : nodeIOPairs) {
			p->copyToNode(globalNode);
		}
	}

	void NodeIOObject::copyToGlobalFromCalculator(Node *globalNode)
	{
		// we could make it a bit more efficient by weeding out unnecessary pairs
		for (auto p : nodeIOPairs) {
			if (!p->localVar->isEquation) {
				p->copyToNode(globalNode);
			}
		}
	}
}
