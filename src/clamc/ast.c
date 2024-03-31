#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "message.h"

void Parameter_init(Parameter* param)
{
	String_init(&param->name);
	Type_init(&param->type);
}

void FuncDecl_init(FuncDecl* func)
{
	Vector_init(&func->block, sizeof(Statement));
	Vector_init(&func->parameters, sizeof(Parameter));
	String_init(&func->name);
	String_init(&func->resType.name);
}

void FuncDecl_destroy(FuncDecl* func)
{
	Vector_destroy(&func->block);
	Vector_destroy(&func->parameters);
}

void VarDecl_destroy(VarDecl* vd)
{
	if (vd->initExpr)
		Expression_destroy(vd->initExpr);
}

void Declaration_init(Declaration* decl)
{
	memset(decl, 0, sizeof(Declaration));
}

void Declaration_destroy(Declaration* decl)
{
	switch (decl->type)
	{
	case DECL_TYPE_FUNCTION:
		FuncDecl_destroy(&decl->function);
		break;

	case DECL_TYPE_VARIANT:
		VarDecl_destroy(&decl->variant);
		break;
	}
}

Expression* Expression_create(ExprType type, SourceLocation* loc)
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
	Expression* expr = Expression_create(type, &token->location);

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

	case EXPR_TYPE_BOOL:
		expr->boolExpr = token->value == TOKEN_VALUE_TRUE;
		break;
	}

	return expr;
}

Expression* Expression_createCall(SourceLocation* loc, Expression* func)
{
	Expression* expr = Expression_create(EXPR_TYPE_CALL, loc);
	expr->callExpr.func = func;
	Vector_init(&expr->callExpr.args, sizeof(Expression));
	return expr;
}

Expression* Expression_createIdent(SourceLocation* loc, Token* token)
{
	Expression* expr = Expression_create(EXPR_TYPE_IDENT, loc);
	expr->identExpr = token->literal;
	return expr;
}

Expression* Expression_createUnary(SourceLocation* loc, ExprType type, Expression* right)
{
	Expression* expr = Expression_create(type, loc);
	expr->unaryExpr = right;
	return expr;
}

Expression* Expression_createBinary(SourceLocation* loc, ExprType type, Expression* left, Expression* right)
{
	Expression* expr = Expression_create(type, loc);
	expr->binaryExpr.leftExpr = left;
	expr->binaryExpr.rightExpr = right;
	return expr;
}

void Expression_destroy(Expression* expr)
{
	switch (expr->type)
	{
	case EXPR_TYPE_CALL:
		Expression_destroy(expr->callExpr.func);
		free(expr->callExpr.func);
		for (int i = 0; i < expr->callExpr.args.size; ++i)
			free(Vector_get(&expr->callExpr.args, i));
		Vector_destroy(&expr->callExpr.args);
		break;

	case EXPR_TYPE_PLUS:
	case EXPR_TYPE_MINUS:
	case EXPR_TYPE_NOT:
	case EXPR_TYPE_NEG:
		Expression_destroy(expr->unaryExpr);
		free(expr->unaryExpr);
		break;

	case EXPR_TYPE_ADD:
	case EXPR_TYPE_SUB:
	case EXPR_TYPE_MUL:
	case EXPR_TYPE_DIV:
	case EXPR_TYPE_MOD:
	case EXPR_TYPE_NE:
	case EXPR_TYPE_EQ:
	case EXPR_TYPE_LT:
	case EXPR_TYPE_LE:
	case EXPR_TYPE_GT:
	case EXPR_TYPE_GE:
	case EXPR_TYPE_AND:
	case EXPR_TYPE_OR:
	case EXPR_TYPE_BITAND:
	case EXPR_TYPE_BITOR:
	case EXPR_TYPE_XOR:
	case EXPR_TYPE_LSHIFT:
	case EXPR_TYPE_RSHIFT:
		Expression_destroy(expr->binaryExpr.leftExpr);
		Expression_destroy(expr->binaryExpr.rightExpr);
		free(expr->binaryExpr.leftExpr);
		free(expr->binaryExpr.rightExpr);
		break;

	case EXPR_TYPE_COND:
		Expression_destroy(expr->condExpr.expr1);
		Expression_destroy(expr->condExpr.expr2);
		Expression_destroy(expr->condExpr.expr3);
		free(expr->condExpr.expr1);
		free(expr->condExpr.expr2);
		free(expr->condExpr.expr3);
		break;
	}

	free(expr);
}

bool Expression_contains(Expression* expr, ExprType exprType)
{
	if (expr->type == exprType)
		return true;

	switch (expr->type)
	{
	case EXPR_TYPE_CALL:
		if (Expression_contains(expr->callExpr.func, exprType))
			return true;
		for (int i = 0; i < expr->callExpr.args.size; ++i)
		{
			Expression* arg = Vector_get(&expr->callExpr.args, i);
			if (Expression_contains(arg, exprType))
				return true;
		}
		return false;

	case EXPR_TYPE_PLUS:
	case EXPR_TYPE_MINUS:
	case EXPR_TYPE_NOT:
	case EXPR_TYPE_NEG:
		return Expression_contains(expr->unaryExpr, exprType);

	case EXPR_TYPE_ADD:
	case EXPR_TYPE_SUB:
	case EXPR_TYPE_MUL:
	case EXPR_TYPE_DIV:
	case EXPR_TYPE_MOD:
	case EXPR_TYPE_NE:
	case EXPR_TYPE_EQ:
	case EXPR_TYPE_LT:
	case EXPR_TYPE_LE:
	case EXPR_TYPE_GT:
	case EXPR_TYPE_GE:
	case EXPR_TYPE_AND:
	case EXPR_TYPE_OR:
	case EXPR_TYPE_BITAND:
	case EXPR_TYPE_BITOR:
	case EXPR_TYPE_XOR:
	case EXPR_TYPE_LSHIFT:
	case EXPR_TYPE_RSHIFT:
		if (Expression_contains(expr->binaryExpr.leftExpr, exprType))
			return true;
		return Expression_contains(expr->binaryExpr.rightExpr, exprType);

	case EXPR_TYPE_COND:
		if (Expression_contains(expr->condExpr.expr1, exprType))
			return true;
		if (Expression_contains(expr->condExpr.expr2, exprType))
			return true;
		return Expression_contains(expr->condExpr.expr3, exprType);
	}

	return false;
}

void Statement_init(Statement* stat)
{
	memset(stat, 0, sizeof(Statement));
}

void Statement_destroy(Statement* stat)
{
	switch (stat->type)
	{
	case STATEMENT_TYPE_DECLARATION:
		Declaration_destroy(&stat->declaration);
		break;

	case STATEMENT_TYPE_ASSIGN:
	case STATEMENT_TYPE_ADD_ASSIGN:
	case STATEMENT_TYPE_SUB_ASSIGN:
	case STATEMENT_TYPE_MUL_ASSIGN:
	case STATEMENT_TYPE_DIV_ASSIGN:
	case STATEMENT_TYPE_MOD_ASSIGN:
		Expression_destroy(stat->assign.leftExpr);
		Expression_destroy(stat->assign.rightExpr);
		break;

	case STATEMENT_TYPE_INC:
	case STATEMENT_TYPE_DEC:
		Expression_destroy(stat->expr);
		break;

	case STATEMENT_TYPE_IF:
		Expression_destroy(stat->ifStat.condition);
		Statement_destroy(stat->ifStat.statement);
		free(stat->ifStat.statement);
		break;
	
	case STATEMENT_TYPE_EXPRESSION:
		if (stat->returnExpr)
			Expression_destroy(stat->returnExpr);
		break;

	case STATEMENT_TYPE_COMPOUND:
		Vector_destroy(&stat->compound);
		break;
	}
}

Statement* Statement_alloc()
{
	Statement* stat = malloc(sizeof(Statement));
	return stat;
}
