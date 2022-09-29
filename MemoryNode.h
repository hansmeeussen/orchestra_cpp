#pragma once

#include "ExpressionNode.h"

// Forward class declarations:
namespace orchestracpp { class ExpressionNode; }

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
namespace orchestracpp
{

	/**
	 *
	 * @author jmeeussen
	 */
	class MemoryNode : public ExpressionNode
	{ //-----------------------------------------

	public:
		bool needsEvaluation = true;
	private:
		double lastValue = 0;
	public:
		ExpressionNode *child = nullptr;
		int nrReferences = 0;

		virtual ~MemoryNode()
		{
		//	delete child;
		}

		MemoryNode(ExpressionNode *child);

		double evaluate() override;

		void setDependentMemoryNode(MemoryNode *parent) override;

		bool constant() override;

		/**
		 * This memory node is redundant if: - the child is constant - there is only
		 * one parent and no attached variable Be careful, this method can be called
		 * by multiple dependent nodes Optimization should only happen once!
		 */
		ExpressionNode *optimize(Parser* parser) override;

	//    public String toString() {
	//        return "M(" + child.toString() + ")";
	//    }

	}; //-----------------------------------------------------------------------------------------

}
