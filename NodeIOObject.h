#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include "NodeIOPair.h"
#include "NodeType.h"
#include "VarGroup.h"
#include "Node.h"

namespace orchestracpp { class NodeIOPair; }

namespace orchestracpp
{

	/**
	 * A NodeIObject takes care of the data transfer between a Node and a
	 * Calculator, NodeStream, Graph. During the Link phase iopairs are added for
	 * each variable that is present in a node as well as in a Calculator. During
	 * the calculation phase these iopairs are used to copy data from the node to
	 * the calculator, and back, after the calculator has finished. The values of
	 * the local variables of the calculator are thus overwritten with the global
	 * values of the node variables with the same name.
	 *
	 *
	 * @author Hans Meeussen 1999 November 1999: March 2000: Improved efficiency by
	 * using linked list instead of standard vector. July 2002: December 2011:
	 * Removed linked list
	 */
	class NodeIOObject final
	{

	private:
		//std::vector<NodeIOPair*> nodeIOPairs = std::vector<NodeIOPair*>();
		// we now (July 2025) have two different lists for two different directions
		std::vector<NodeIOPair*> toLocalList = std::vector<NodeIOPair*>();
		std::vector<NodeIOPair*> toGlobalList = std::vector<NodeIOPair*>();
		/**
		 * During the link phase a new IOObject is created for each variable that
		 * exists both in the node and in the local set of variables. These
		 * IOObjects are added to nodeIOPairs.
		 */

	public:
		virtual ~NodeIOObject()
		{
			for (auto pair : toLocalList) {
				delete pair;
			}
			for (auto pair : toGlobalList) {
				delete pair;
			}
		}

		/**
		 * This constructor is used in the calculator
		 */
		NodeIOObject(const std::string &prefix, VarGroup *variables, NodeType *nodeType);

		/**
		 * Copy the global values to the local variables
		 */

		void copyToLocal(Node *globalNode);

		/**
		 * Copy the local values to the global node variables So the output list is
		 * different from the input list. Can we automatically detect variable that
		 * are not changed by the calculator? Be careful, this method is also used
		 * by copying from cells to nodestreams etc.!
		 */
		// Called from: 
		// NodeInStream
		// SimpleCalculator
		// TransportCalculator
		// these have no equation variables
		void copyToGlobal(Node *globalNode);

		void copyToGlobalFromCalculator(Node *globalNode);
	};

}
