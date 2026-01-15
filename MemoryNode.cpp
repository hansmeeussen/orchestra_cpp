#include "MemoryNode.h"
#include "Parser.h"

namespace orchestracpp
{

	MemoryNode::MemoryNode(ExpressionNode *child)
	{
		this->child = child;
	}

	double MemoryNode::evaluate()
	{
		if (needsEvaluation)
		{
			lastValue = child->evaluate();
			needsEvaluation = false;
		}
		return lastValue;
	}

	void MemoryNode::setDependentMemoryNode(MemoryNode *parent)
	{
		child->setDependentMemoryNode(parent);
		if (!dependentMemoryNodesDone) {
			child->setDependentMemoryNode(this);
			dependentMemoryNodesDone = true;
		}
	}

	bool MemoryNode::constant()
	{
 		return (child->constant());
	}

	ExpressionNode *MemoryNode::optimize(Parser* parser)
	{
		if (!isoptimized) // we do this only once
		{
			child = child->optimize(parser);
			isoptimized = true;

			if (child->constant())
			{
				child = NumberNode::createNumberNode(evaluate(), parser);
			}
		}


		if (child->constant() || nrReferences <= 1) {
			return child; // this effectively removes  memorynode
		}

		return this;
	}
}
