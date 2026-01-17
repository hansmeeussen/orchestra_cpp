#pragma once

#include "ExpressionNode.h"
#include <string>
#include <map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <cmath>
#include "OrchestraException.h"
#include "VarGroup.h"

namespace orchestracpp { class ParserException; }
namespace orchestracpp { class MemoryNode; }
namespace orchestracpp { class ExpressionString; }
namespace orchestracpp { class ParserStringTokenizer; }
namespace orchestracpp { class BExpressionNode; }
namespace orchestracpp { class ExpressionNode; }
namespace orchestracpp { class NumberNode; }
namespace orchestracpp { class VarGroup; }

namespace orchestracpp
{

	/**
	 * An Expression Parser and Evaluator. The Parser class creates an expression
	 * graph from a set of variables and expression strings. The nodes of the
	 * expression graph (operators and operands) are represented by the different
	 * expression node types defined below.
	 *
	 * The expression (nodes) are recursively optimized for constant expressions and
	 * strength reduction. The memory nodes store intermediate results for nodes
	 * with more than one parent. The setParent method, links all input variables
	 * with their dependent memory nodes, so if an input variable changes value, all
	 * dependent memory nodes are notified to recalculate their values.
	 *
	 *
	 * @author Hans Meeussen, June 2002, January 2003
	 *
	 * sqrt(<expression>)
	 * abs(<expression>) log(<expression>) log10(<expression>)
	 * exp(<expression>) sin(<expression>) cos(<expression>) tan(<expression>)
	 * sinh(<expression>) cosh(<expression>) tanh(<expression>) atan(<expression>)
	 * max(<expression>,<expression>) min(<expression>,<expression>)
	 * if(<condition>,<expression>,<expression>) // condition =  <expression>[<,>, <=,
	 * >=, ==]<expression>
	 *
	         print(<expression>)
	 *
	 */
	class Parser final
	{
		//64211CP538VGB47
		//11627

	public:

		std::map<std::string, NumberNode*> constants; // static set of unique constants

		std::vector<ExpressionNode*> allExpressionNodes;
		std::vector<BExpressionNode*> allBExpressionNodes;

		ExpressionNode* newNode(ExpressionNode* tmp);
		BExpressionNode* newBNode(BExpressionNode* tmp);

		~Parser()
		{
			//std::cout << "Deleting all "<< allExpressionNodes.size() <<" expression nodes " << std::endl;
			for (auto n : allExpressionNodes) {
				delete n;
			}
			for (auto n : allBExpressionNodes) {
				delete n;
			}

		}

		static void main(std::string args[]);

	private:
		VarGroup * variables; // The variables group that the parser operates on.

		/*
		 * A static method to evaluate a given expresson in a string
		 */
	public:
		static double evaluate(const std::string &expression) /*throw(ParserException)*/;

		Parser();

	    Parser(VarGroup * variables);

		/**
		 * Add an expression to the graph and return the resulting parent (result)
		 * expression node. call node.optimize() to optimize and node.evaluate() to
		 * evaluate
		 */
	private:
		ExpressionNode *parse(const std::string &expression) /*throw(ParserException)*/;

		/**
		 * todo: We should detect when a variable is re-used as expression input
		 * AFTER it was used as an expression result How: 1) Mark each variable when
		 * it is used as expression result usedAsExpresionResult = true; 2) Check
		 * each variable for this flag when used as expression input. 3) Notify when
		 * this happens.
		 */

		/**
		 * Add an expression to the expression graph.
		 */
	public:
		void addExpression(ExpressionString *expression)/* throw(ParserException)*/;

		/**
		 * Recursive set of parse methods 1) Parse a sum
		 */
	private:
		ExpressionNode *parseSum(ParserStringTokenizer *stream)/* throw(ParserException)*/;

		/**
		 * 2) Parse a term
		 */
		ExpressionNode *parseTerm(ParserStringTokenizer *stream) /*throw(ParserException)*/;

		/**
		 * 3) Parse an exponent
		 */
		ExpressionNode *parseExponent(ParserStringTokenizer *stream)/* throw(ParserException)*/;

		/**
		 * 4) Parse an element.
		 */
		ExpressionNode *parseElement(ParserStringTokenizer *stream)/* throw(ParserException)*/;

		BExpressionNode *parseBElement(ParserStringTokenizer *stream)/* throw(ParserException)*/;
	}; //-----------------------------------------------------------------------------------------

	/**
	 * A node representing a number operand.
	 * The static collection of constants is shared by multiple calculators!
	 * So access needs to be synchronized!
	 * Adapted 27/12/2018 07:17
	 * Probably it is better to store the map of constants in the  parser object, so no synchronization is required
	 * to do this we need a pointer to the parser in each expresion node
	 */
	class NumberNode final : public ExpressionNode
	{ 

	private:
		const double value;

    public:
		//static std::map<std::string, NumberNode*> constants; // static set of unique constants

		/**
		 * This method creates all new nodes
		 */
		static NumberNode *createNumberNode(const std::string &name, Parser* parser);

		static NumberNode *createNumberNode(double value, Parser* parser);

		NumberNode(const std::string &name, Parser* parser);
		
	public:
		double evaluate() override;

		void setDependentMemoryNode(MemoryNode *parent) override;

		bool constant() override;

		ExpressionNode *optimize(Parser* parser) override;

	//    public String toString() {
	//        return Double.toString(value);
	//    }
	}; //-----------------------------------------------------------------------------------------

	/**
	 * An absolute value node
	 */
	class AbsNode final : public ExpressionNode
	{ //------------------------------------------

	private:
		ExpressionNode *child;

	public:
	
		~AbsNode()
		{
		}

		AbsNode(ExpressionNode *child);

		void setDependentMemoryNode(MemoryNode *parent) override;

		double evaluate() override;

		bool constant() override;

		ExpressionNode *optimize(Parser* parser) override;

	//    public String toString() {
	//        return ("abs(" + child.toString() + ")");
	//    }
	}; //-----------------------------------------------------------------------------------------

	/**
	 * A unary minus node
	 */
	class UMinNode final : public ExpressionNode
	{ //------------------------------------------

	private:
		ExpressionNode *child;

	public:
	    ~UMinNode()
		{
		}

		UMinNode(ExpressionNode *child);

		void setDependentMemoryNode(MemoryNode *parent) override;

		double evaluate() override;

		bool constant() override;

		ExpressionNode *optimize(Parser* parser) override;

	//    public String toString() {
	//        return ("-" + child.toString());
	//    }
	}; //-----------------------------------------------------------------------------------------

	class PlusNode final : public ExpressionNode
	{ //------------------------------------------

	private:
		ExpressionNode *left;
		ExpressionNode *right;

	public:
		~PlusNode()
		{
		}

		PlusNode(ExpressionNode *left, ExpressionNode *right);

		double evaluate() override;

		void setDependentMemoryNode(MemoryNode *parent) override;

		bool constant() override;

		ExpressionNode *optimize(Parser* parser) override;

	//    public String toString() {
	//        return ("(" + left.toString() + "+" + right.toString() + ")");
	//    }
	}; //-----------------------------------------------------------------------------------------

	class MinusNode final : public ExpressionNode
	{ //-------------------------------------------

	private:
		ExpressionNode *left;
		ExpressionNode *right;

	public:
		~MinusNode()
		{
		}

		MinusNode(ExpressionNode *left, ExpressionNode *right);

		double evaluate() override;

		void setDependentMemoryNode(MemoryNode *parent) override;

		bool constant() override;

		ExpressionNode *optimize(Parser* parser) override;

	//    public String toString() {
	//        return ("(" + left.toString() + "-" + right.toString() + ")");
	//    }
	}; //-----------------------------------------------------------------------------------------

	class TimesNode final : public ExpressionNode
	{ //-----------------------------------------

	private:
		ExpressionNode *left;
		ExpressionNode *right;

	public:
		~TimesNode()
		{
		}

		TimesNode(ExpressionNode *left, ExpressionNode *right);

		double evaluate() override;

		void setDependentMemoryNode(MemoryNode *parent) override;

		bool constant() override;

		ExpressionNode *optimize(Parser* parser) override;

	//    public String toString() {
	//        return ("(" + left.toString() + "*" + right.toString() + ")");
	//    }
	}; //-----------------------------------------------------------------------------------------

	class DivideNode final : public ExpressionNode
	{ //----------------------------------------

	private:
		ExpressionNode *left;
		ExpressionNode *right;

	public:
		~DivideNode()
		{
		}

		DivideNode(ExpressionNode *left, ExpressionNode *right);

		double evaluate() override;

		bool constant() override;

		void setDependentMemoryNode(MemoryNode *parent) override;

		ExpressionNode *optimize(Parser* parser) override;

	//    public String toString() {
	//        return ("(" + left.toString() + "/" + right.toString() + ")");
	//    }
	}; //-----------------------------------------------------------------------------------------

	class MaxNode final : public ExpressionNode
	{ //----------------------------------------

	private:
		ExpressionNode *left;
		ExpressionNode *right;

	public:
		~MaxNode()
		{
		}

		MaxNode(ExpressionNode *left, ExpressionNode *right);

		double evaluate() override;

		bool constant() override;

		void setDependentMemoryNode(MemoryNode *parent) override;

		ExpressionNode *optimize(Parser* parser) override;

	//    public String toString() {
	//        return ("max(" + left.toString() + "," + right.toString() + ")");
	//    }
	}; //-----------------------------------------------------------------------------------------

	class MinimumNode final : public ExpressionNode
	{ //----------------------------------------

	private:
		ExpressionNode *left;
		ExpressionNode *right;

		std::unordered_set<MemoryNode*> dependentChildren = std::unordered_set<MemoryNode*>();

	public:
		~MinimumNode()
		{
		}

		MinimumNode(ExpressionNode *left, ExpressionNode *right);

		double evaluate() override;

		bool constant() override;

		void setDependentMemoryNode(MemoryNode *parent) override;

		ExpressionNode *optimize(Parser* parser) override;

	//    public String toString() {
	//        return ("min(" + left.toString() + "," + right.toString() + ")");
	//    }
	}; //-----------------------------------------------------------------------------------------


	class PowerNode final : public ExpressionNode
	{ //-----------------------------------------

	private:
		ExpressionNode *left;
		ExpressionNode *right;

	public:
		~PowerNode()
		{
		}

		PowerNode(ExpressionNode *left, ExpressionNode *right);

		double evaluate() override;

		void setDependentMemoryNode(MemoryNode *parent) override;

		bool constant() override;

		ExpressionNode *optimize(Parser* parser) override;

	//    public String toString() {
	//        return ("(" + left.toString() + "^" + right.toString() + ")");
	//    }
	}; //-----------------------------------------------------------------------------------------

	class Power10Node final : public ExpressionNode
	{ //-----------------------------------------

	private:
		ExpressionNode *right;

	public:
		~Power10Node()
		{
		}

		Power10Node(ExpressionNode *right);

		double evaluate() override;

		void setDependentMemoryNode(MemoryNode *parent) override;

		bool constant() override;

		ExpressionNode *optimize(Parser* parser) override;

	//    public String toString() {
	//        return ("(10^" + right.toString() + ")");
	//    }
	}; //-----------------------------------------------------------------------------------------

	/**
	 * The if node returns the left or right child expression depending on the
	 * result of the condition. true returns left, false returns right. May be we
	 * should change the boolean expression nodes into ordinary ones, true is
	 * anything other than zero 1 = true 0 = false
	 */
	class IfNode final : public ExpressionNode
	{ //--------------------------------------------

	private:
		BExpressionNode *condition;
		ExpressionNode *left;
		ExpressionNode *right;
		std::unordered_set<MemoryNode*> dependentChildren;

	public:
		~IfNode()
		{
//			delete condition;
		}

		IfNode(BExpressionNode *condition, ExpressionNode *left, ExpressionNode *right);

		double evaluate() override;

		void setDependentMemoryNode(MemoryNode *parent) override;

		bool constant() override;

		ExpressionNode *optimize(Parser* parser) override;

	//    public String toString() {
	//        return ("(if(" + condition + "," + left.toString() + "," + right.toString() + ")");
	//    }
	}; //-----------------------------------------------------------------------------------------
	

	class Function1Node : public ExpressionNode
	{ //----------------------------------

	public:
		ExpressionNode *child;
						
		Function1Node(ExpressionNode *child);

        double evaluate()override = 0; ;

		void setDependentMemoryNode(MemoryNode *parent)override;

		bool constant()override;

		ExpressionNode *optimize(Parser* parser)override;
		
        ~Function1Node(){}
		
	}; //-----------------------------------------------------------------------------------------


	// the MemoryNode class was moved to its own source file, as it is used as public class
	// and java 13 did not accept this anymore in this place


	class SqrtNode final : public Function1Node
	{ //----------------------------------------------

	public:
		SqrtNode(ExpressionNode *child);

		double evaluate() override;

	//    public String toString() {
	//        return ("sqrt(" + child.toString() + ")");
	//    }
	}; //-----------------------------------------------------------------------------------------

	class SqrNode final : public Function1Node
	{ //----------------------------------------------

	public:
		SqrNode(ExpressionNode* child);

		double evaluate() override;

		//    public String toString() {
		//        return ("sqrt(" + child.toString() + ")");
		//    }
	}; //-----------------------------------------------------------------------------------------

	class LogNode final : public Function1Node
	{

	public:
		LogNode(ExpressionNode *child);

		double evaluate() override;

	//    public String toString() {
	//        return ("log(" + child.toString() + ")");
	//    }
	};

	class Log10Node final : public Function1Node
	{

	public:
		Log10Node(ExpressionNode *child);

		double evaluate() override;

	//    public String toString() {
	//        return ("log10(" + child.toString() + ")");
	//    }
	};

	class ExpNode final : public Function1Node
	{

	public:
		ExpNode(ExpressionNode *child);

		double evaluate() override;

	//    public String toString() {
	//        return ("exp(" + child.toString() + ")");
	//    }
	};

	class SinNode final : public Function1Node
	{

	public:
		SinNode(ExpressionNode *child);

		double evaluate() override;

	//    public String toString() {
	//        return ("sin(" + child.toString() + ")");
	//    }
	};

	class CosNode final : public Function1Node
	{

	public:
		CosNode(ExpressionNode *child);

		double evaluate() override;

	//    public String toString() {
	//        return ("cos(" + child.toString() + ")");
	//    }
	};

	class TanNode final : public Function1Node
	{

	public:
		TanNode(ExpressionNode *child);

		double evaluate() override;

	//    public String toString() {
	//        return ("tan(" + child.toString() + ")");
	//    }
	};

	class SinhNode final : public Function1Node
	{

	public:
		SinhNode(ExpressionNode *child);

		double evaluate() override;

	//    public String toString() {
	//        return ("sinh(" + child.toString() + ")");
	//    }
	};

	class CoshNode final : public Function1Node
	{

	public:
		CoshNode(ExpressionNode *child);

		double evaluate() override;

	//    public String toString() {
	//        return ("cosh(" + child.toString() + ")");
	//    }
	};

	class TanhNode final : public Function1Node
	{

	public:
		TanhNode(ExpressionNode *child);

		double evaluate() override;

	//    public String toString() {
	//        return ("tanh(" + child.toString() + ")");
	//    }
	};

	class ATanNode final : public Function1Node
	{

	public:
		ATanNode(ExpressionNode *child);

		double evaluate() override;

	//    public String toString() {
	//        return ("atan(" + child.toString() + ")");
	//    }
	};

	class PrintNode final : public Function1Node
	{

	public:
		PrintNode(ExpressionNode *child);

		double evaluate() override;

	//    public String toString() {
	//        return ("print(" + child.toString() + ")");
	//    }

	};

	/**
	 * Boolean expression nodes return true or false
	 * and are used as part of the IfNode
	 */
/*
	class BExpressionNode
	{

	public:
		ExpressionNode *left;
		ExpressionNode *right;
	private:
		bool isoptimized = false;

	public:
		virtual ~BExpressionNode(){}

		BExpressionNode(ExpressionNode *left, ExpressionNode *right);

		virtual bool evaluate() = 0;

		virtual void setDependentMemoryNode(MemoryNode *parent);

		virtual bool constant();

		virtual BExpressionNode *optimize(Parser* parser);
	};

	*/

	class EQNode final : public BExpressionNode
	{

	public:
		EQNode(ExpressionNode *left, ExpressionNode *right);

		bool evaluate() override;

		//std::string toString();
	};

	class Not final : public BExpressionNode
	{

	private:
		BExpressionNode *child;

	public:
		~Not(){}

		Not(BExpressionNode *child);

		bool evaluate() override;
	};

	class LTNode final : public BExpressionNode
	{

	public:
		~LTNode() {}

		LTNode(ExpressionNode *left, ExpressionNode *right);

		bool evaluate() override;
	};

	class GTNode final : public BExpressionNode
	{

	public:
		~GTNode() {}

		GTNode(ExpressionNode *left, ExpressionNode *right);

		bool evaluate() override;

	//    public String toString() {
	//        return ("(" + left.toString() + ">" + right.toString() + ")");
	//    }

	};

	class LENode final : public BExpressionNode
	{

	public:
		~LENode() {}

		LENode(ExpressionNode *left, ExpressionNode *right);

		bool evaluate() override;
	};

	class GENode final : public BExpressionNode
	{

	public:
		~GENode() {}


		GENode(ExpressionNode *left, ExpressionNode *right);

		bool evaluate() override;
	};


}
