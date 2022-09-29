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
		child->setDependentMemoryNode(this);
	}

	bool MemoryNode::constant()
	{
 		return (child->constant());
	}

	ExpressionNode *MemoryNode::optimize(Parser* parser)
	{
		if (isoptimized)
		{
			return this;
		}
		isoptimized = true;

		child = child->optimize(parser);

		if (child->constant())
		{
			child = NumberNode::createNumberNode(evaluate(),parser);
			return child;
		}

		if (nrReferences <= 1)
		{ // this effectively removes  memorynode
			return child;
		}

		return this;
	}
}
