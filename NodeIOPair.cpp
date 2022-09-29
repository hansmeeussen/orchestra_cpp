#include "NodeIOPair.h"
#include "Var.h"
#include "Node.h"

namespace orchestracpp
{

	NodeIOPair::NodeIOPair(Var *local, int global)
	{
		localVar = local;
		nodeVarIndex = global;
		local->setConstant(false);
		local->usedForIO = true;
	}

	void NodeIOPair::copyFromNode(Node *node)
	{
	   localVar->setValue(node->getvalue(nodeVarIndex));
	}

	void NodeIOPair::copyToNode(Node *node)
	{
	   node->setValue(nodeVarIndex, localVar->getValue());
	}
}
