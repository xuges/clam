#include <string.h>

#include "parser.h"
#include "message.h"

static void _Parser_toplevel(Parser* p);
static Declaration _Parser_declaration(Parser* p);
static Type _Parser_typeDesc(Parser* p);
static Vector _Parser_parameterList(Parser* p);
static Statement _Parser_statement(Parser* p);
static Vector _Parser_compoundStatement(Parser* p);
static Expression* _Parser_expression(Parser* p);
static Expression* _Parser_assignExpression(Parser* p);
static Expression* _Parser_unaryExpression(Parser* p);
static Expression* _Parser_additiveExpression(Parser* p);
static Expression* _Parser_multiplicativeExpression(Parser* p);
static Expression* _Parser_postfixExpression(Parser* p);
static Expression* _Parser_primaryExpression(Parser* p);
static Vector _Parser_argumentList(Parser* p);
static Token* _Parser_until(Parser* p, TokenValue value, const char* msg);
static Token* _Parser_expect(Parser* p, TokenValue value, const char* msg);


bool Parser_init(Parser* p)
{
	Module_init(&p->module);
	p->lex = NULL;
	return true;
}

void Parser_destroy(Parser* p)
{
	Module_destroy(&p->module);
}

Module* Parser_translate(Parser* p, Lexer* lex)
{
	p->lex = lex;

	Lexer_next(p->lex);
	while (Lexer_peek(p->lex)->value != TOKEN_VALUE_EOF)
		_Parser_toplevel(p);

	p->lex = NULL;
	return &p->module;
}

void _Parser_toplevel(Parser* p)
{
	Declaration decl = _Parser_declaration(p);
	Module_addDeclaration(&p->module, &decl);
}

Declaration _Parser_declaration(Parser* p)
{
	Declaration decl;
	Declaration_init(&decl);

	Token* token = Lexer_peek(p->lex);
	if (token->value == TOKEN_VALUE_EOF)
		error(&p->lex->location, "unexpected EOF");

	if (token->value == TOKEN_VALUE_EXPORT)
	{
		decl.exported = true;
		decl.location = token->location;
		Lexer_next(p->lex);
	}

	if (!decl.exported)
		decl.location = token->location;

	//type
	decl.baseType = _Parser_typeDesc(p);

	_Parser_expect(p, TOKEN_VALUE_IDENT, "expected declaration name");
	decl.name = token->literal;

	Lexer_next(p->lex);
	switch (token->value)
	{
	case TOKEN_VALUE_LP:
		//function
		decl.type = DECL_TYPE_FUNCTION;
		decl.function.parameters = _Parser_parameterList(p);

		Lexer_next(p->lex);
		//TODO: check interface
		decl.function.block = _Parser_compoundStatement(p);
		return decl;

	case TOKEN_VALUE_SEM:
		//variant
		decl.type = DECL_TYPE_VARIANT;
		decl.variant.initExpr = NULL;
		break;

	case TOKEN_VALUE_ASSIGN:
		//variant with init expression
		decl.type = DECL_TYPE_VARIANT;
		Lexer_next(p->lex);
		decl.variant.initExpr = _Parser_expression(p);
		_Parser_expect(p, TOKEN_VALUE_SEM, "expected ';'");
		break;

	default:
		error(&token->location, "unexpected '" String_FMT "'", String_arg(token->literal));
	}

	Lexer_next(p->lex);
	return decl;
}

Type _Parser_typeDesc(Parser* p)
{
	Type type = errorType;
	Token* token = Lexer_peek(p->lex);

	switch (token->type)
	{
	case TOKEN_TYPE_KEYWORD_TYPE:
		switch (token->value)
		{
		case TOKEN_VALUE_VOID:
			type = voidType;
			break;

		case TOKEN_VALUE_INT:
			type = intType;
			break;
		}
		break;

	default:
		type = errorType;
		break;
	}

	Lexer_next(p->lex);
	return type;
}

Vector _Parser_parameterList(Parser* p)
{
	Vector list;
	Vector_init(&list, sizeof(Parameter));

	Token* token = Lexer_peek(p->lex);
	Lexer_next(p->lex);

	Parameter param;

	while (token->value != TOKEN_VALUE_EOF && token->value != TOKEN_VALUE_RP)
	{
		Parameter_init(&param);

		//type
		param.type = _Parser_typeDesc(p);

		//name
		token = _Parser_expect(p, TOKEN_VALUE_IDENT, "expected parameter name");
		param.name = token->literal;
		Lexer_next(p->lex);

		Vector_add(&list, &param);

		if (token->value == TOKEN_VALUE_COMMA)
			Lexer_next(p->lex);
		else if (token->value != TOKEN_VALUE_RP)
			error(&token->location, "unexpected '" String_FMT "'", String_arg(token->literal));
	}
	_Parser_expect(p, TOKEN_VALUE_RP, "expected ')'");

	return list;
}

Vector _Parser_compoundStatement(Parser* p)
{
	Token* token = Lexer_peek(p->lex);
	_Parser_expect(p, TOKEN_VALUE_LC, "expected '{'");

	Vector cs;
	Vector_init(&cs, sizeof(Statement));

	Statement stat;
	Statement_init(&stat);

	Lexer_next(p->lex);
	while (token->value != TOKEN_VALUE_EOF && token->value != TOKEN_VALUE_RC)
	{
		stat = _Parser_statement(p);
		Vector_add(&cs, &stat);
	}

	_Parser_expect(p, TOKEN_VALUE_RC, "expected '}'");
	Lexer_next(p->lex);

	return cs;
}

Statement _Parser_statement(Parser* p)
{
	Statement stat;
	Statement_init(&stat);

	Token* token = Lexer_peek(p->lex);
	switch (token->value)
	{
	case TOKEN_VALUE_LC:
		stat.type = STATEMENT_TYPE_COMPOUND;
		stat.location = token->location;
		stat.compound = _Parser_compoundStatement(p);
		break;

	case TOKEN_VALUE_RETURN:
		stat.type = STATEMENT_TYPE_RETURN;
		stat.location = token->location;
		if (Lexer_next(p->lex)->value != TOKEN_VALUE_SEM)
			stat.returnExpr = _Parser_expression(p);
		_Parser_expect(p, TOKEN_VALUE_SEM, "expected ';'");
		Lexer_next(p->lex);
		break;

	case TOKEN_VALUE_SEM:
		stat.type = STATEMENT_TYPE_EMPTY;
		stat.location = token->location;
		Lexer_next(p->lex);
		break;

	case TOKEN_VALUE_EXPORT:  //syntax correct but semantic incorrect
		stat.type = STATEMENT_TYPE_DECLARATION;
		stat.location = token->location;
		stat.declaration = _Parser_declaration(p);
		break;

	//case TOKEN_VALUE_IDENT:  //TODO: support custom type

	default:
		stat.location = token->location;

		if (token->type == TOKEN_TYPE_KEYWORD_TYPE)
		{
			stat.type = STATEMENT_TYPE_DECLARATION;
			stat.declaration = _Parser_declaration(p);
			break;
		}

		stat.type = STATEMENT_TYPE_EXPRESSION;
		stat.expr = _Parser_expression(p);
		_Parser_expect(p, TOKEN_VALUE_SEM, "expected ';'");
		Lexer_next(p->lex);
	}

	return stat;
}

Expression* _Parser_expression(Parser* p)
{
	return _Parser_assignExpression(p);
}

Expression* _Parser_assignExpression(Parser* p)
{
	Token* token = Lexer_peek(p->lex);
	SourceLocation loc = token->location;

	Expression* left = _Parser_additiveExpression(p);

	ExprType exprType;
	switch (token->value)
	{
	case TOKEN_VALUE_ASSIGN:
		exprType = EXPR_TYPE_ASSIGN;
		break;

	default:
		return left;
	}

	Lexer_next(p->lex);

	Expression* right = _Parser_expression(p);
	return Expression_createAssign(&loc, exprType, left, right);
}

Expression* _Parser_additiveExpression(Parser* p)
{
	Token* token = Lexer_peek(p->lex);
	SourceLocation loc = token->location;

	Expression* left = _Parser_multiplicativeExpression(p);
	
	while (token->value == TOKEN_VALUE_ADD || token->value == TOKEN_VALUE_SUB)
	{
		ExprType exprType;
		switch (token->value)
		{
		case TOKEN_VALUE_ADD:
			exprType = EXPR_TYPE_ADD;
			break;

		case TOKEN_VALUE_SUB:
			exprType = EXPR_TYPE_SUB;
			break;
		}

		Lexer_next(p->lex);

		Expression* right = _Parser_multiplicativeExpression(p);
		left = Expression_createBinary(&loc, exprType, left, right);
		token = Lexer_peek(p->lex);
		loc = token->location;
	}

	return left;
}

Expression* _Parser_multiplicativeExpression(Parser* p)
{
	Token* token = Lexer_peek(p->lex);
	SourceLocation loc = token->location;

	Expression* left = _Parser_unaryExpression(p);

	while (token->value == TOKEN_VALUE_STAR)
	{
		ExprType exprType;
		switch (token->value)
		{
		case TOKEN_VALUE_STAR:
			exprType = EXPR_TYPE_MUL;
			break;
		}

		Lexer_next(p->lex);

		Expression* right = _Parser_unaryExpression(p);
		left = Expression_createBinary(&loc, exprType, left, right);
		token = Lexer_peek(p->lex);
		loc = token->location;
	}

	return left;
}

Expression* _Parser_unaryExpression(Parser* p)
{
	Token* token = Lexer_peek(p->lex);
	SourceLocation loc = token->location;

	ExprType exprType;
	switch (token->value)
	{
	case TOKEN_VALUE_ADD:
		exprType = EXPR_TYPE_PLUS;
		break;

	case TOKEN_VALUE_SUB:
		exprType = EXPR_TYPE_MINUS;
		break;

	default:
		return _Parser_postfixExpression(p);
	}

	Lexer_next(p->lex);

	Expression* right = _Parser_unaryExpression(p);
	return Expression_createUnary(&loc, exprType, right);
}

Expression* _Parser_postfixExpression(Parser* p)
{
	Expression* expr;
	Token* token = Lexer_peek(p->lex);
	SourceLocation loc = token->location;

	expr = _Parser_primaryExpression(p);

	token = Lexer_peek(p->lex);
	while (token->value == TOKEN_VALUE_LP)
	{
		switch (token->value)
		{
		case TOKEN_VALUE_LP:
			expr = Expression_createCall(&loc, expr);
			expr->callExpr.args = _Parser_argumentList(p);
			break;
		}
		token = Lexer_peek(p->lex);
	}

	return expr;
}

Expression* _Parser_primaryExpression(Parser* p)
{
	Expression* expr = NULL;
	Token* token = Lexer_peek(p->lex);
	switch (token->value)
	{
	case TOKEN_VALUE_LITERAL_INT:
		expr = Expression_createLiteral(EXPR_TYPE_INT, token);
		Lexer_next(p->lex);
		break;
	case TOKEN_VALUE_IDENT:
		expr = Expression_createIdent(&token->location, token);
		Lexer_next(p->lex);
		break;
	}

	return expr;
}

Vector _Parser_argumentList(Parser* p)
{
	_Parser_expect(p, TOKEN_VALUE_LP, "expected '('");
	Lexer_next(p->lex);

	Vector list;
	Vector_init(&list, sizeof(Expression));

	Token* token = Lexer_peek(p->lex);
	while (token->value != TOKEN_VALUE_EOF && token->value != TOKEN_VALUE_RP)
	{
		Expression* expr = _Parser_expression(p);
		Vector_add(&list, expr);

		token = Lexer_peek(p->lex);
		if (token->value == TOKEN_VALUE_COMMA)
			Lexer_next(p->lex);
		else if (token->value != TOKEN_VALUE_RP)
			error(&token->location, "unexpected '" String_FMT "'", String_arg(token->literal));
	}

	_Parser_expect(p, TOKEN_VALUE_RP, "expected ')'");
	Lexer_next(p->lex);

	return list;
}

Token* _Parser_until(Parser* p, TokenValue value, const char* msg)
{
	Token* token = Lexer_peek(p->lex);
	while (token->value != value && token->value != TOKEN_VALUE_EOF)
		token = Lexer_next(p->lex);
	if (token->value == TOKEN_VALUE_EOF)
	{
		error(&token->location, msg);
		return NULL;
	}

	return token;
}

Token* _Parser_expect(Parser* p, TokenValue value, const char* msg)
{
	Token* token = Lexer_peek(p->lex);
	if (token->value == value)
		return token;

	error(&token->location, msg);
	return NULL;
}
