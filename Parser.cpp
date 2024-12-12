#include "Parser.h"
#include "ParserStringTokenizer.h"
#include "ExpressionString.h"
#include "MemoryNode.h"

#include "OrchestraException.h"
#include "stringhelper.h"

#include <sstream>

namespace orchestracpp
{

	/* Todo for C++ version:
	* The parser creates new expression nodes of different types during the parsing process.
	* These nodes can become unlinked from their parents during optimization, so are not deleted when the parser is destructed.
	* This is not a big problem, as this only occurs during initialization/ optimization, but should be handled correctly.
	* 
	* This issue is resolved by registering all newly created nodes with the newNode methods below.
	* 
	*/

	void Parser::main(std::string args[])
	{
		// a main method just to test the parser
		try
		{
		   std::cout << Parser::evaluate("2.3+3.5") << std::endl;
		}
		catch (const ParserException &pe)
		{
			std::cout << pe.what() << std::endl;
		}
	}

	/* This method registers all new expressionnodes so they can be deleted upon destruction of the Parser object
	*/
	ExpressionNode* Parser::newNode(ExpressionNode* tmp) {
		allExpressionNodes.push_back(tmp);
		return tmp;
	}

	BExpressionNode* Parser::newBNode(BExpressionNode* tmp) {
		allBExpressionNodes.push_back(tmp);
		return tmp;
	}

	double Parser::evaluate(const std::string &expression) //throw(ParserException)
	{
		Parser tmpParser;
		return tmpParser.parse(expression)->optimize(&tmpParser)->evaluate();
	}

	Parser::Parser()  : variables(nullptr)
	{
	}

	Parser::Parser(VarGroup *variables2) : variables(variables2)
	{
	}

	ExpressionNode *Parser::parse(const std::string &expression)// throw(ParserException) 
	{		
		ParserStringTokenizer tokenizer(StringHelper::trim(expression));
		ExpressionNode *node = parseSum(&tokenizer);

		if (tokenizer.hasMoreTokens())
		{
            std::string tmp =   "Found a redundant:   " + tokenizer.nextToken() + "   in the expression: " + expression;
			throw OrchestraException(tmp);		
		}

		return node;
	}


	void Parser::addExpression(ExpressionString *expression)// throw(ParserException)
	{

		Var *resultVariable = variables->get(expression->getResultVariableName());

		if (resultVariable == nullptr){
			throw ParserException("Could not find variable: " + expression->resultVariableName + " when parsing " + expression->expression);
		}

		//resultVariable->usedAsExpressionResult = true;

		// each new result variable gets a memory node
		// this memory node may be removed during optimization when this variable is only used once.
		resultVariable->memory = new MemoryNode(parse(expression->getExpression()));
		newNode(resultVariable->memory); // register this node for deletion

	}

	ExpressionNode *Parser::parseSum(ParserStringTokenizer *stream) //throw(ParserException)
	{
		ExpressionNode *anchor = parseTerm(stream);
		while (stream->hasMoreTokens() && stream->match("+-"))
		{
			if (stream->match("+"))
			{
				stream->consume();
				anchor = newNode(new PlusNode(anchor, parseTerm(stream)));
			}
			else
			{
				stream->consume();
				anchor = newNode(new MinusNode(anchor, parseTerm(stream)));
			}
		}

		return anchor;
	}

	ExpressionNode *Parser::parseTerm(ParserStringTokenizer *stream)// throw(ParserException)
	{
		ExpressionNode *anchor = parseExponent(stream);

		while (stream->hasMoreTokens() && stream->match("*/"))
		{
			if (stream->match("*"))
			{
				stream->consume();
				anchor = newNode(new TimesNode(anchor, parseExponent(stream)));
			}
			else
			{
				stream->consume();
				anchor = newNode(new DivideNode(anchor, parseExponent(stream)));
			}
		}
		return anchor;
	}

	ExpressionNode *Parser::parseExponent(ParserStringTokenizer *stream)// throw(ParserException)
	{
		ExpressionNode *anchor = parseElement(stream);

		while (stream->hasMoreTokens() && stream->match("^"))
		{
			stream->consume();
			anchor = newNode(new PowerNode(anchor, parseElement(stream)));
		}

		return anchor;
	}

	ExpressionNode *Parser::parseElement(ParserStringTokenizer *stream) //throw(ParserException)
	{

		if ((stream->hasMoreTokens() && stream->match("(")))
		{
			stream->consume();
			ExpressionNode *leftExpression = parseSum(stream);
			if (!stream->hasMoreTokens() && (stream->getCurrentToken() == ""))
			{
				throw OrchestraException("')' expected!" +stream->getExpression());
			}
			if (!stream->match(")"))
			{
				throw ParserException("')' expected!"+stream->getExpression());
			}
			else
			{
				stream->consume();
			}
			return leftExpression;
		}
		else if (stream->match("-"))
		{
			stream->consume();
			return (newNode(new UMinNode(parseElement(stream))));
		}
		else if (stream->isNumber())
		{
			ExpressionNode *leftExpression = NumberNode::createNumberNode(stream->getCurrentToken(), this);
			stream->consume();
			return leftExpression;
		}
		else if (stream->equals("sqrt"))
		{
			stream->consume();
			return newNode(new SqrtNode(parseElement(stream)));
		}
		else if (stream->equals("log"))
		{
			stream->consume();
			return newNode(new LogNode(parseElement(stream)));
		}
		else if (stream->equals("abs"))
		{
			stream->consume();
			return newNode(new AbsNode(parseElement(stream)));
		}
		else if (stream->equals("log10"))
		{
			stream->consume();
			return newNode(new Log10Node(parseElement(stream)));
		}
		else if (stream->equals("exp"))
		{
			stream->consume();
			return newNode(new ExpNode(parseElement(stream)));
		}
		else if (stream->equals("sin"))
		{
			stream->consume();
			return newNode(new SinNode(parseElement(stream)));
		}
		else if (stream->equals("cos"))
		{
			stream->consume();
			return newNode(new CosNode(parseElement(stream)));
		}
		else if (stream->equals("tan"))
		{
			stream->consume();
			return newNode(new TanNode(parseElement(stream)));
		}
		else if (stream->equals("sinh"))
		{
			stream->consume();
			return newNode(new SinhNode(parseElement(stream)));
		}
		else if (stream->equals("cosh"))
		{
			stream->consume();
			return newNode(new CoshNode(parseElement(stream)));
		}
		else if (stream->equals("tanh"))
		{
			stream->consume();
			return newNode(new TanhNode(parseElement(stream)));
		}
		else if (stream->equals("atan"))
		{
			stream->consume();
			return newNode(new ATanNode(parseElement(stream)));
		}
		else if (stream->equals("print"))
		{
			stream->consume();
			return newNode(new PrintNode(parseElement(stream)));
		}
		else if (stream->equals("if"))
		{
			stream->consume();
			stream->matchAndConsume("(");
			BExpressionNode *condition = parseBElement(stream);
			stream->matchAndConsume(",");
			ExpressionNode *e1 = parseSum(stream);
			stream->matchAndConsume(",");
			ExpressionNode *e2 = parseSum(stream);
			stream->matchAndConsume(")");
			return newNode(new IfNode(condition, e1, e2));
		}
		else if (stream->equals("max"))
		{
			stream->consume();
			stream->matchAndConsume("(");
			ExpressionNode *e1 = parseSum(stream);
			stream->matchAndConsume(",");
			ExpressionNode *e2 = parseSum(stream);
			stream->matchAndConsume(")");
			return newNode(new MaxNode(e1, e2));
		}
		else if (stream->equals("min"))
		{
			stream->consume();
			stream->matchAndConsume("(");
			ExpressionNode *e1 = parseSum(stream);
			stream->matchAndConsume(",");
			ExpressionNode *e2 = parseSum(stream);
			stream->matchAndConsume(")");
			return newNode(new MinimumNode(e1, e2));
		}


		// This identifier must be a variable name
		Var *variable = stream->isVariable(variables);

		if (variable != nullptr)
		{
			stream->consume();

			if (variable->memory == nullptr)
			{
				// a variable gets an attached memory node if it is used as intermediate result.
				// If not, simply return the variable
				return variable;
			}
			else
			{
				variable->memory->nrReferences++;
				return variable->memory;
			}
		}
		
		throw ParserException("Variable or number expected but found: '" + stream->getCurrentToken() + "'" +stream->getExpression());
	}

	BExpressionNode *Parser::parseBElement(ParserStringTokenizer *stream)// throw(ParserException)
	{
		if (stream->match("("))
		{
			stream->consume();
			BExpressionNode *bExpression = parseBElement(stream);
			if (!stream->match(")"))
			{
				throw ParserException("')' expected!"+stream->getExpression());
			}
			else
			{
				stream->consume();
			}
			return bExpression;
		}
		else if (stream->match("!"))
		{
			stream->consume();
			return newBNode(new Not((parseBElement(stream))));
		}

		ExpressionNode *left = parseSum(stream);

		if (stream->match("<"))
		{
			stream->consume();
			if (stream->match("="))
			{
				stream->consume();
				return newBNode(new LENode(left, parseSum(stream)));
			}
			else
			{
				return newBNode(new LTNode(left, parseSum(stream)));
			}
		}
		else if (stream->match(">"))
		{
			stream->consume();
			if (stream->match("="))
			{
				stream->consume();
				return newBNode(new GENode(left, parseSum(stream)));
			}
			else
			{
				return newBNode(new GTNode(left, parseSum(stream)));
			}
		}
		else if (stream->match("="))
		{
			stream->consume();
			if (stream->match("="))
			{
				stream->consume();
				return new EQNode(left, parseSum(stream));
			}
		}

		return nullptr;
	}


//    std::map<std::string, NumberNode*> NumberNode::constants;
	//std::map<std::string, NumberNode*> Parser::constants;

	NumberNode *NumberNode::createNumberNode(const std::string &nameIn, Parser* parser)
	{
		NumberNode *tmpNode;
		/* convert string to double and back to assure standard format of string (1.0 1 1.000) */
		double tmpNumber = std::stod(nameIn);
        // convert double to string
	    // be careful: standard C++ dtos has too low accuracy!
		std::string name = StringHelper::doubleToString(tmpNumber, 12);
	
		if (parser->constants.find(name) != parser->constants.end())
		{ //  this constant already exists,
			tmpNode = parser->constants[name];
		}
		else
		{ // does not exist, create new one and add to constants
			tmpNode = new NumberNode(name,parser);
			parser->newNode(tmpNode); // register as expression node for deletion
			parser->constants.emplace(name, tmpNode);
			//std::cout << std::setprecision(12) << nameIn << " " << tmpNumber << " " << name << std::endl;

		}
		return tmpNode;
	}

	NumberNode *NumberNode::createNumberNode(double value, Parser* parser)
	{
		// be careful: standard C++ dtos has too low accuracy!
		return createNumberNode(StringHelper::doubleToString(value, 12), parser);
	}

	NumberNode::NumberNode(const std::string &name, Parser* parser) : value(std::stod(name))
	{
	}

	double NumberNode::evaluate()
	{
		return value;
	}

	void NumberNode::setDependentMemoryNode(MemoryNode *parent)
	{
	}

	bool NumberNode::constant()
	{
		return true;
	}

	ExpressionNode *NumberNode::optimize(Parser* parser)
	{
		return this;
	}

	AbsNode::AbsNode(ExpressionNode *child)
	{
		this->child = child;
	}

	void AbsNode::setDependentMemoryNode(MemoryNode *parent)
	{
		child->setDependentMemoryNode(parent);
	}

	double AbsNode::evaluate()
	{
		return std::abs(child->evaluate());
	}

	bool AbsNode::constant()
	{
		return child->constant();
	}

	ExpressionNode *AbsNode::optimize(Parser* parser)
	{
		if (isoptimized)
		{
			return this;
		}
		isoptimized = true;

		child = child->optimize(parser);
		if (constant())
		{
			return NumberNode::createNumberNode(evaluate(),parser);
		}
		return this;
	}

	UMinNode::UMinNode(ExpressionNode *child)
	{
		this->child = child;
	}

	void UMinNode::setDependentMemoryNode(MemoryNode *parent)
	{
		child->setDependentMemoryNode(parent);
	}

	double UMinNode::evaluate()
	{
		return -(child->evaluate());
	}

	bool UMinNode::constant()
	{
		return child->constant();
	}

	ExpressionNode *UMinNode::optimize(Parser* parser)
	{
		if (isoptimized)
		{
			return this;
		}

		isoptimized = true;

		child = child->optimize(parser);

		if (constant())
		{
			return NumberNode::createNumberNode(evaluate(),parser);
		}

		return this;
	}

	PlusNode::PlusNode(ExpressionNode *left, ExpressionNode *right)
	{
		this->left = left;
		this->right = right;
	}

	double PlusNode::evaluate()
	{
		return left->evaluate() + right->evaluate();
	}

	void PlusNode::setDependentMemoryNode(MemoryNode *parent)
	{
		left->setDependentMemoryNode(parent);
		right->setDependentMemoryNode(parent);
	}

	bool PlusNode::constant()
	{
		return (left->constant() && right->constant());
	}

	ExpressionNode *PlusNode::optimize(Parser* parser)
	{

		if (isoptimized)
		{
			return this;
		}
		isoptimized = true;

		left = left->optimize(parser);
		right = right->optimize(parser);

		if (constant())
		{
			return NumberNode::createNumberNode(evaluate(),parser);
		}

		if (right->constant())
		{
			if (right->evaluate() == 0.0)
			{
				return left;
			}
		}

		if (left->constant())
		{
			if (left->evaluate() == 0.0)
			{
				return right;
			}
		}

		return this;
	}

	MinusNode::MinusNode(ExpressionNode *left, ExpressionNode *right)
	{
		this->left = left;
		this->right = right;
	}

	double MinusNode::evaluate()
	{
		return left->evaluate() - right->evaluate();
	}

	void MinusNode::setDependentMemoryNode(MemoryNode *parent)
	{
		left->setDependentMemoryNode(parent);
		right->setDependentMemoryNode(parent);
	}

	bool MinusNode::constant()
	{
		return (left->constant() && right->constant());
	}

	ExpressionNode *MinusNode::optimize(Parser* parser)
	{
		if (isoptimized)
		{
			return this;
		}
		isoptimized = true;

		left = left->optimize(parser);
		right = right->optimize(parser);

		if (constant())
		{
			return NumberNode::createNumberNode(evaluate(),parser);
		}

		if (right->constant())
		{
			if (right->evaluate() == 0.0)
			{
				return left;
			}
		}

		if (left->constant())
		{
			if (left->evaluate() == 0.0)
			{
				ExpressionNode * tmpNode = new UMinNode(right);
				return tmpNode->optimize(parser);
			}
		}

		return this;
	}

	TimesNode::TimesNode(ExpressionNode *left, ExpressionNode *right)
	{
		this->left = left;
		this->right = right;
	}

	double TimesNode::evaluate()
	{
		return left->evaluate() * right->evaluate();
	}

	void TimesNode::setDependentMemoryNode(MemoryNode *parent)
	{
		left->setDependentMemoryNode(parent);
		right->setDependentMemoryNode(parent);
	}

	bool TimesNode::constant()
	{
		return (left->constant() && right->constant());
	}

	ExpressionNode *TimesNode::optimize(Parser* parser)
	{
		if (isoptimized)
		{
			return this;
		}
		isoptimized = true;

		left = left->optimize(parser);
		right = right->optimize(parser);

		if (constant())
		{
			return NumberNode::createNumberNode(evaluate(),parser);
		}

		if (right->constant())
		{
			double rightvalue = right->evaluate();

			if (rightvalue == 0.0)
			{
				return NumberNode::createNumberNode(0.0,parser);
			}

			if (rightvalue == 1.0)
			{
				return left;
			}
		}

		if (left->constant())
		{
			double leftvalue = left->evaluate();

			if (leftvalue == 0.0)
			{
				return NumberNode::createNumberNode(0.0,parser);
			}

			if (leftvalue == 1.0)
			{
				return right;
			}

		}

		return this;
	}

	DivideNode::DivideNode(ExpressionNode *left, ExpressionNode *right)
	{
		this->left = left;
		this->right = right;
	}

	double DivideNode::evaluate()
	{
		return left->evaluate() / right->evaluate();
	}

	bool DivideNode::constant()
	{
		return (left->constant() && right->constant());
	}

	void DivideNode::setDependentMemoryNode(MemoryNode *parent)
	{
		left->setDependentMemoryNode(parent);
		right->setDependentMemoryNode(parent);
	}

	ExpressionNode *DivideNode::optimize(Parser* parser)
	{
		if (isoptimized)
		{
			return this;
		}
		isoptimized = true;

		left = left->optimize(parser);
		right = right->optimize(parser);

		if (constant())
		{
			return NumberNode::createNumberNode(evaluate(),parser);
		}

		if (left->constant() && (left->evaluate() == 0.0))
		{
			return NumberNode::createNumberNode(0.0,parser);
		}

		if (right->constant() && (right->evaluate() == 1.0))
		{
			return left;
		}

		return this;
	}

	MaxNode::MaxNode(ExpressionNode *left, ExpressionNode *right)
	{
		this->left = left;
		this->right = right;
	}

	double MaxNode::evaluate()
	{
		double l = left->evaluate();
		double r = right->evaluate();

		if (l > r)
		{
			return l;
		}
		else
		{
			return r;
		}
	}

	bool MaxNode::constant()
	{
		return (left->constant() && right->constant());
	}

	void MaxNode::setDependentMemoryNode(MemoryNode *parent)
	{
		left->setDependentMemoryNode(parent);
		right->setDependentMemoryNode(parent);
	}

	ExpressionNode *MaxNode::optimize(Parser* parser)
	{
		if (isoptimized)
		{
			return this;
		}
		isoptimized = true;

		left = left->optimize(parser);
		right = right->optimize(parser);

		if (constant())
		{
			return NumberNode::createNumberNode(evaluate(),parser);
		}

		return this;
	}

	MinimumNode::MinimumNode(ExpressionNode *left, ExpressionNode *right)
	{
		this->left = left;
		this->right = right;
	}

	double MinimumNode::evaluate()
	{
		double l = left->evaluate();
		double r = right->evaluate();

		if (l < r)
		{
			return l;
		}
		else
		{
			return r;
		}
	}

	bool MinimumNode::constant()
	{
		return (left->constant() && right->constant());
	}

	void MinimumNode::setDependentMemoryNode(MemoryNode *parent)
	{
		if (std::find(dependentChildren.begin(), dependentChildren.end(), parent) != dependentChildren.end())
		{
			return;
		}
		dependentChildren.insert(parent);
		left->setDependentMemoryNode(parent);
		right->setDependentMemoryNode(parent);
	}

	ExpressionNode *MinimumNode::optimize(Parser* parser)
	{
		if (isoptimized)
		{
			return this;
		}
		isoptimized = true;

		left = left->optimize(parser);
		right = right->optimize(parser);

		if (constant())
		{
			return NumberNode::createNumberNode(evaluate(),parser);
		}
		return this;
	}

	PowerNode::PowerNode(ExpressionNode *left, ExpressionNode *right)
	{
		this->left = left;
		this->right = right;
	}

	double PowerNode::evaluate()
	{
		return std::pow(left->evaluate(), right->evaluate());
	}

	void PowerNode::setDependentMemoryNode(MemoryNode *parent)
	{
		left->setDependentMemoryNode(parent);
		right->setDependentMemoryNode(parent);
	}

	bool PowerNode::constant()
	{
		return (left->constant() && right->constant());
	}

	ExpressionNode *PowerNode::optimize(Parser* parser)
	{

		if (isoptimized)
		{
			return this;
		}
		isoptimized = true;

		left = left->optimize(parser);
		right = right->optimize(parser);
		

		if (constant())
		{
			return NumberNode::createNumberNode(evaluate(),parser);
		}


		if (left->constant())
		{
			double leftvalue = left->evaluate();
			if (leftvalue == 0.0)
			{
				return NumberNode::createNumberNode(0.0, parser);
			}
			if (leftvalue == 1.0)
			{
				return NumberNode::createNumberNode(1.0, parser);
			}
			if (leftvalue == 10.0)
			{
				ExpressionNode * tempNode = parser->newNode(new Power10Node(right));
				return tempNode->optimize(parser);
			}
		}

		//return this; //********************************


		if (right->constant())
		{
			double rightvalue = right->evaluate();

			if (rightvalue == 0.0)
			{
				return NumberNode::createNumberNode(1.0,parser);
			}

			if (rightvalue == 1.0)
			{
				return left;
			}

			if (rightvalue == 2.0)
			{
				return parser->newNode(new SqrNode(left));
			}

			if (rightvalue == -1.0)
			{
				ExpressionNode * tempNode = parser->newNode(new DivideNode(NumberNode::createNumberNode(1.0, parser), left));
				return tempNode->optimize(parser);
			}

		}

		return this;
	}

	Power10Node::Power10Node(ExpressionNode *right)
	{
		this->right = right;
	}

	double Power10Node::evaluate()
	{
		return std::pow(10, right->evaluate());
	}

	void Power10Node::setDependentMemoryNode(MemoryNode *parent)
	{
		right->setDependentMemoryNode(parent);
	}

	bool Power10Node::constant()
	{
		return (right->constant());
	}

	ExpressionNode *Power10Node::optimize(Parser* parser)
	{
		if (isoptimized)
		{
			return this;
		}
		isoptimized = true;

		right = right->optimize(parser);

		if (constant())
		{
			return NumberNode::createNumberNode(evaluate(),parser);
		}

		return this;
	}

	IfNode::IfNode(BExpressionNode *condition, ExpressionNode *left, ExpressionNode *right)
	{
		this->condition = condition;
		this->left = left;
		this->right = right;
		//dependentChildren = std::unordered_set<MemoryNode*>();
	}

	double IfNode::evaluate()
	{
		if (condition->evaluate())
		{
			return left->evaluate();
		}
		else
		{
			return right->evaluate();
		}
	}

	void IfNode::setDependentMemoryNode(MemoryNode *parent)
	{
		if (std::find(dependentChildren.begin(), dependentChildren.end(), parent) != dependentChildren.end())
		{
			return;
		}
		dependentChildren.insert(parent);
		condition->setDependentMemoryNode(parent);
		left->setDependentMemoryNode(parent);
		right->setDependentMemoryNode(parent);
	}

	bool IfNode::constant()
	{
		//return false;
		//return (condition->constant() && left->constant() && right->constant());

		if (condition->constant()) {
			if (condition->evaluate()) {
				return left->constant();
			}
			else {
				return right->constant();
			}
		}
		return false;

	}

	ExpressionNode *IfNode::optimize(Parser* parser)
	{
		if (isoptimized)
		{
			return this;
		}
		condition->optimize(parser);

		isoptimized = true;

		if (condition->constant()) {
			if (condition->evaluate()) {
				if (left->constant()) {
					return NumberNode::createNumberNode(left->evaluate(), parser);
				}
				return  left->optimize(parser);
			}
			else {
				if (right->constant()) {
					return NumberNode::createNumberNode(right->evaluate(), parser);
				}
				return  right->optimize(parser);
			}
		}
		else {
			if (left->constant() && right->constant()) {
				if (left->evaluate() == right->evaluate()) {
					return NumberNode::createNumberNode(right->evaluate(), parser);
				}
			}

			left = left->optimize(parser);
			right = right->optimize(parser);
			return this;
		}

	}

	
	//************* The function1Node methods
	Function1Node::Function1Node(ExpressionNode *child)
	{
		this->child = child;
	}

	void Function1Node::setDependentMemoryNode(MemoryNode *parent)
	{
		child->setDependentMemoryNode(parent);
	}

	bool Function1Node::constant()
	{
		return (child->constant());
	}

	ExpressionNode *Function1Node::optimize(Parser* parser)
	{
		if (isoptimized)
		{
			return this;
		}
		isoptimized = true;

		child = child->optimize(parser);

		if (constant())
		{
			return NumberNode::createNumberNode(evaluate(), parser);
		}
		return this;
	}

	SqrNode::SqrNode(ExpressionNode* child) : Function1Node(child)
	{
	}

	double SqrNode::evaluate()
	{
		return std::pow(child->evaluate(),2);
	}

	SqrtNode::SqrtNode(ExpressionNode *child) : Function1Node(child)
	{
	}

	double SqrtNode::evaluate()
	{
		return std::sqrt(child->evaluate());
	}

	LogNode::LogNode(ExpressionNode *child) : Function1Node(child)
	{
	}

	double LogNode::evaluate()
	{
		return std::log(child->evaluate());
	}

	Log10Node::Log10Node(ExpressionNode *child) : Function1Node(child)
	{
	}

	double Log10Node::evaluate()
	{
		return std::log10(child->evaluate());
	}

	ExpNode::ExpNode(ExpressionNode *child) : Function1Node(child)
	{
	}

	double ExpNode::evaluate()
	{
		return std::exp(child->evaluate());
	}

	SinNode::SinNode(ExpressionNode *child) : Function1Node(child)
	{
	}

	double SinNode::evaluate()
	{
		return std::sin(child->evaluate());
	}

	CosNode::CosNode(ExpressionNode *child) : Function1Node(child)
	{
	}

	double CosNode::evaluate()
	{
		return std::cos(child->evaluate());
	}

	TanNode::TanNode(ExpressionNode *child) : Function1Node(child)
	{
	}

	double TanNode::evaluate()
	{
		return std::tan(child->evaluate());
	}

	SinhNode::SinhNode(ExpressionNode *child) : Function1Node(child)
	{
	}

	double SinhNode::evaluate()
	{
		return std::sinh(child->evaluate());
	}

	CoshNode::CoshNode(ExpressionNode *child) : Function1Node(child)
	{
	}

	double CoshNode::evaluate()
	{
		return std::cosh(child->evaluate());
	}

	TanhNode::TanhNode(ExpressionNode *child) : Function1Node(child)
	{
	}

	double TanhNode::evaluate()
	{
		return std::tanh(child->evaluate());
	}

	ATanNode::ATanNode(ExpressionNode *child) : Function1Node(child)
	{
	}

	double ATanNode::evaluate()
	{
		return std::atan(child->evaluate());
	}

	PrintNode::PrintNode(ExpressionNode *child) : Function1Node(child)
	{
	}

	double PrintNode::evaluate()
	{
		double tmp = child->evaluate();
		return tmp;
	}

	BExpressionNode::BExpressionNode(ExpressionNode *left, ExpressionNode *right)
	{
		this->left = left;
		this->right = right;
	}

	void BExpressionNode::setDependentMemoryNode(MemoryNode *parent)
	{
		left->setDependentMemoryNode(parent);
		right->setDependentMemoryNode(parent);
	}

	bool BExpressionNode::constant()
	{
		return (left->constant() && right->constant());
	}

	//BExpressionNode *BExpressionNode::optimize(Parser* parser)
    void BExpressionNode::optimize(Parser* parser)
	{
		if (isoptimized)
		{
			return;// this;
		}
		isoptimized = true;
		left = left->optimize(parser);
		right = right->optimize(parser);
		return;// this;
	}

	EQNode::EQNode(ExpressionNode *left, ExpressionNode *right) : BExpressionNode(left, right)
	{
	}

	bool EQNode::evaluate()
	{
		return (left->evaluate() == right->evaluate());
	}


	Not::Not(BExpressionNode *child) : BExpressionNode(nullptr, nullptr)
	{
		this->child = child;
	}

	bool Not::evaluate()
	{
		return !child->evaluate();
	}

	LTNode::LTNode(ExpressionNode *left, ExpressionNode *right) : BExpressionNode(left, right)
	{
	}

	bool LTNode::evaluate()
	{
		return (left->evaluate() < right->evaluate());
	}

	GTNode::GTNode(ExpressionNode *left, ExpressionNode *right) : BExpressionNode(left, right)
	{
	}

	bool GTNode::evaluate()
	{
		return (left->evaluate() > right->evaluate());
	}

	LENode::LENode(ExpressionNode *left, ExpressionNode *right) : BExpressionNode(left, right)
	{
	}

	bool LENode::evaluate()
	{
		return (left->evaluate() <= right->evaluate());
	}

	GENode::GENode(ExpressionNode *left, ExpressionNode *right) : BExpressionNode(left, right)
	{
	}

	bool GENode::evaluate()
	{
		return (left->evaluate() >= right->evaluate());
	}
}
