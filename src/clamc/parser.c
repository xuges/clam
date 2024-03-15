#include <string.h>

#include "parser.h"
#include "message.h"

static void _Parser_toplevel(Parser* p);
static void _Parser_declaration(Parser* p);
static Vector _Parser_parameterList(Parser* p);
static Vector _Parser_statementBlock(Parser* p);
static Statement _Parser_statement(Parser* p);
static Expression* _Parser_expression(Parser* p);
static Expression* _Parser_postfixExpression(Parser* p);
static Expression* _Parser_primaryExpression(Parser* p);
static Token* _Parser_until(Parser* p, TokenValue value, const char* msg);
static Token* _Parser_expect(Parser* p, TokenValue value, const char* msg);


bool Parser_init(Parser* p)
{
	Module_init(&p->module);

	memset(&p->decl, 0, sizeof(Declaration));
	memset(&p->stat, 0, sizeof(Statement));

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
	Declaration_init(&p->decl);
	_Parser_declaration(p);
	Module_addDeclaration(&p->module, &p->decl);
}

void _Parser_declaration(Parser* p)
{
	Token* token = Lexer_peek(p->lex);
	if (token->value == TOKEN_VALUE_EOF)
	{
		error(&p->lex->location, "unexpected EOF");
		return;
	}

	if (token->value == TOKEN_VALUE_EXPORT)
	{
		p->decl.exported = true;
		p->decl.location = token->location;
		Lexer_next(p->lex);
	}

	switch (token->type)
	{
	case TOKEN_TYPE_KEYWORD_TYPE:
	case TOKEN_TYPE_IDENT:
		if (!p->decl.exported)
		{
			p->decl.location = token->location;
		}

		p->decl.baseType.name = token->literal;
		p->decl.baseType.value = token->value;
		break;

	default:
		error(&token->location, "unexpected '" String_FMT "'", String_arg(token->literal));
	}

	token = Lexer_next(p->lex);
	_Parser_expect(p, TOKEN_VALUE_IDENT, "expected declaration name");
	p->decl.name = token->literal;

	Lexer_next(p->lex);
	switch (token->value)
	{
	case TOKEN_VALUE_LP:
		//function
		p->decl.type = DECL_TYPE_FUNCTION;
		//TODO: parse parameter list
		Lexer_next(p->lex);
		_Parser_expect(p, TOKEN_VALUE_RP, "expected ')'");

		Lexer_next(p->lex);
		//TODO: check interface
		p->decl.function.block = _Parser_statementBlock(p);
		return;

	case TOKEN_VALUE_SEM:
		//variant
		p->decl.type = DECL_TYPE_VARIANT;
		p->decl.variant.initExpr = NULL;
		break;

	case TOKEN_VALUE_ASSIGN:
		//variant with init expression
		p->decl.type = DECL_TYPE_VARIANT;
		Lexer_next(p->lex);
		p->decl.variant.initExpr = _Parser_expression(p);
		_Parser_expect(p, TOKEN_VALUE_SEM, "expected ';'");
		break;

	default:
		error(&token->location, "unexpected '" String_FMT "'", String_arg(token->literal));
	}

	Lexer_next(p->lex);
}

Vector _Parser_parameterList(Parser* p)
{
	//Parameter param;

	Token* token = Lexer_peek(p->lex);
	while (token->value != TOKEN_VALUE_EOF && token->value != TOKEN_VALUE_RP)
	{
		//TODO: parse parameter type and name
	}
}

Vector _Parser_statementBlock(Parser* p)
{
	Statement stat;
    Vector block;
	Vector_init(&block, sizeof(Statement));
	Vector_reserve(&block, 8);

	_Parser_expect(p, TOKEN_VALUE_LC, "expected '{'");
	Lexer_next(p->lex);

	Token* token = Lexer_peek(p->lex);
	while (token->value != TOKEN_VALUE_EOF && token->value != TOKEN_VALUE_RC)
	{
		if (token->value == TOKEN_VALUE_SEM)
		{
			token = Lexer_next(p->lex);
			continue;
		}
		else
		{
			stat = _Parser_statement(p);
			Vector_add(&block, &stat);
		}
	}

	_Parser_expect(p, TOKEN_VALUE_RC, "expected '}'");
	Lexer_next(p->lex);

	return block;
}

Statement _Parser_statement(Parser* p)
{
	Statement stat;
	Token* token = Lexer_peek(p->lex);
	switch (token->value)
	{
	case TOKEN_VALUE_RETURN:
		stat.location = token->location;
		stat.type = STATEMENT_TYPE_RETURN;
		if (Lexer_next(p->lex)->value != TOKEN_VALUE_SEM)
		{
			stat.type = STATEMENT_TYPE_RETURN_EXPR;
			stat.returnExpr = _Parser_expression(p);
		}
		break;
	default:
		stat.location = token->location;
		stat.type = STATEMENT_TYPE_EXPRESSION;
		stat.expr = _Parser_expression(p);
		_Parser_expect(p, TOKEN_VALUE_SEM, "expected ';'");
		Lexer_next(p->lex);
	}
	return stat;
}

Expression* _Parser_expression(Parser* p)
{
	return _Parser_postfixExpression(p);
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
			token = Lexer_next(p->lex);
			while (token->value != TOKEN_VALUE_EOF && token->value != TOKEN_VALUE_RP)  //skip args TODO: parse arguments
				token = Lexer_next(p->lex);
			_Parser_expect(p, TOKEN_VALUE_RP, "expected ')'");
			Lexer_next(p->lex);
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
