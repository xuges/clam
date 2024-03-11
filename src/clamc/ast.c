#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "message.h"

void FuncDecl_init(FuncDecl* func)
{
	Vector_init(&func->block, sizeof(Statement));
	Vector_init(&func->parameters, sizeof(Parameter));
	String_init(&func->name);
	String_init(&func->resType.name);
	func->resType.value = 0;
}

void Declaration_init(Declaration* decl)
{
	memset(decl, 0, sizeof(Declaration));
}

static Expression* _Expression_create(ExprType type, SourceLocation* loc)
{
	Expression* expr = (Expression*)malloc(sizeof(Expression));
	if (!expr)
		error(NULL, "out of memory");

	expr->type = type;
	expr->location = *loc;
	return expr;
}

Expression* Expression_createLiteral(ExprType type, Token* token)
{
	Expression* expr = _Expression_create(type, &token->location);

	switch (type)
	{
	case EXPR_TYPE_INT:
		switch (token->base)
		{
		case TOKEN_NUM_DEC:
			expr->intExpr = String_toInt(&token->literal, token->base);  //TODO: use safe range
			break;
		}
		break;
	}

	return expr;
}

Expression* Expression_createCall(SourceLocation* loc, Expression* func)
{
	Expression* expr = _Expression_create(EXPR_TYPE_CALL, loc);
	expr->callExpr.func = func;
	return expr;
}

Expression* Expression_createIdent(SourceLocation* loc, Token* token)
{
	Expression* expr = _Expression_create(EXPR_TYPE_IDENT, loc);
	expr->identExpr = token->literal;
	return expr;
}

void Expression_destroy(Expression* expr)
{
	if (expr->type == EXPR_TYPE_CALL)
		Expression_destroy(expr->callExpr.func);

	free(expr);
}

void Statement_destroy(Statement* stat)
{
	switch (stat->type)
	{
	case STATEMENT_TYPE_EXPRESSION:
		Expression_destroy(stat->expr);
		break;

	case STATEMENT_TYPE_RETURN_EXPR:
		Expression_destroy(stat->returnExpr);
		break;
	}


}
