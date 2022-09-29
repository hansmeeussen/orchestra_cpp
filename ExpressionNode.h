#pragma once

namespace orchestracpp { class MemoryNode; }
namespace orchestracpp { class Parser; }

namespace orchestracpp
{

	/**
	 * The base class for all expression nodes in an expression tree
	 */
	class ExpressionNode
	{

		// do we use this or not?
	public:
		bool isoptimized = false;

		/** Evaluate this expression node recursively */
		virtual double evaluate() = 0;

		/** Creates the links to its parents */
		virtual void setDependentMemoryNode(MemoryNode *memoryNode) = 0;

		/** Recursively determines if this node represents a constant expression */
		virtual bool constant() = 0;

		/** Recursively optimizes this expression node and returns an optimized alternative */
		virtual ExpressionNode *optimize(Parser* parser) = 0;
		
		virtual ~ExpressionNode(){}

		/** Returns a recursive string representation of this expression node */
	   // public abstract String toString();

	};


	class BExpressionNode
	{

	public:
		ExpressionNode *left;
		ExpressionNode *right;
	private:
		bool isoptimized = false;

	public:
		virtual ~BExpressionNode() {}

		BExpressionNode(ExpressionNode *left, ExpressionNode *right);

		virtual bool evaluate() = 0;

		virtual void setDependentMemoryNode(MemoryNode *parent);

		virtual bool constant();

		virtual void optimize(Parser* parser);
	};




}
