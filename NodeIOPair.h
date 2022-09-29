#pragma once
#include "Node.h"
#include "Var.h"
#include "NodeType.h"


namespace orchestracpp
{

	/*
	 * A NodeIOPair copies a value of a node variable to 
	 * a local calculator variable and back. 
	 * @author Hans Meeussen 26/7/1999
	 */
	class NodeIOPair
	{
	public:
		int nodeVarIndex = 0; // index to the global Node variable
		Var *localVar = nullptr; // the local variable (calculator or input output stream)

		virtual ~NodeIOPair()
		{
			//delete localVar;
		}

		NodeIOPair(Var *local, int global);

		virtual void copyFromNode(Node *node);

		virtual void copyToNode(Node *node);
	};
}
