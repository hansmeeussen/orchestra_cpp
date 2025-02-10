#pragma once

#include <vector>
#include <cmath>

namespace orchestracpp
{

	class NodeType;

	class Node
	{

	public:
		NodeType *nodeType; // NodeType to which this Node belongs
		double *values;     // values of node variables

		virtual ~Node()
		{
			delete [] values;
		}

		Node(NodeType *nodeType);

		// it is not necessary to synchronize these methods 
		virtual double getvalue(int index);

		virtual void setValue(int index, double value);

		virtual void clone(Node *original);



		// Give the current node values that are in between those of two other nodes.
		// f = fraction between 0 and 1
		virtual void nodeBetween(Node *left, Node *right, double f);

		/**
		 * 
		 * @return a clone of this node
		 */
		virtual Node *clone();

		virtual bool containsNaN();

		// write contents of this node to file
	//    public void dumpCell()throws ReadException, IOException {
	//        NodeOutStream nos = new NodeOutStream();
	//        nos.open(new FileID(null,"celldump.dat"), nodeType);
	//        nos.write(this);
	//        nos.close();      
	//    }
	};

}
