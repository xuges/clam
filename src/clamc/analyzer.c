#include <stdlib.h>
#include <string.h>

#include "analyzer.h"
#include "message.h"

struct Variant
{
	Type type;
	String name;
	int level;
	//TODO; check variant init or not
};
typedef struct Variant Variant;

static void Variant_init(Variant* v)
{
	memset(v, 0, sizeof(Variant));
}

static void _Analyzer_variant(Analyzer* anly, Declaration* decl);
static void _Analyzer_function(Analyzer* anly, Declaration* decl);
static bool _Analyzer_statement(Analyzer* anly, Declaration* decl, Statement* stat);
static bool _Analyzer_ifStatement(Analyzer* anly, Declaration* decl, Statement* stat);
static bool _Analyzer_compoundStatement(Analyzer* anly, Declaration* decl, Statement* stat);
static void _Analyzer_returnStatement(Analyzer* anly, Declaration* decl, Statement* stat);
static void _Analyzer_assignStatement(Analyzer* anly, Statement* stat);
static void _Analyzer_incDecStatement(Analyzer* anly, Statement* stat);
static Type _Analyzer_expression(Analyzer* anly, Expression* exr);
static Type _Analyzer_callExpression(Analyzer* anly, Expression* expr);
static Type _Analyzer_unaryExpression(Analyzer* anly, Expression* expr);
static Type _Analyzer_binaryExpression(Analyzer* anly, Expression* expr);
static bool _Analyzer_checkTypeConvert(Analyzer* anly, Type left, Type right);
static Type _Analyzer_checkTypeOperate(Analyzer* anly, ExprType op, Type* t1, Type* t2, Type* t3);
static bool _Analyzer_checkLvalue(Analyzer* anly, Expression* expr);
static bool _Analyzer_checkZero(Analyzer* anly, Expression* expr);
static Variant* _Analyzer_findVariant(Analyzer* anly, String name);
static Declaration* _Analyzer_findFunction(Analyzer* anly, String name);

void Analyzer_init(Analyzer* anly)
{
	Stack_init(&anly->stack, sizeof(Variant));
	anly->level = 0;
}

void Analyzer_destroy(Analyzer* anly)
{
	Stack_destroy(&anly->stack);
}

void Analyzer_analyze(Analyzer* anly, Module* module)
{
	//TODO: prepare package name
	anly->module = module;

	for (int i = 0; i < module->declarations.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->declarations, i);
		if (decl->type == DECL_TYPE_VARIANT)
			_Analyzer_variant(anly, decl);
	}

	for (int i = 0; i < module->functions.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->functions, i);
		_Analyzer_function(anly, decl);
	}
}

void _Analyzer_variant(Analyzer* anly, Declaration* decl)
{
	if (decl->variant.type.id == TYPE_VOID)
		error(&decl->location, "variant type cannot be 'void'");

	for (int i = anly->stack.size - 1; i >= 0; --i)  //check duplicate
	{
		Variant* v = Vector_get(&anly->stack, i);
		if (v->level == anly->level && String_equalsString(v->name, decl->variant.name))
			error(&decl->location, "variant '" String_FMT "' duplicate", String_arg(decl->name));  //TODO: show existed variant line
	}

	Variant variant;
	
	switch (decl->variant.type.id)
	{
	case TYPE_INT:
	case TYPE_BOOL:
		variant.type = decl->variant.type;
		variant.name = decl->variant.name;
		variant.level = anly->level;
		if (decl->variant.initExpr)
		{
			Type rtype = _Analyzer_expression(anly, decl->variant.initExpr);
			if (!_Analyzer_checkTypeConvert(anly, variant.type, rtype))
				error(&decl->variant.initExpr->location, "rvalue expression type cannot convert to lvalue type");
		}
		break;

	default:
		error(&decl->location, "incorrect variant type '" String_FMT "'", String_arg(decl->variant.type.name));
	}

	Stack_push(&anly->stack, &variant);
}

void _Analyzer_function(Analyzer* anly, Declaration* decl)
{
	anly->level++;
	int lastSize = anly->stack.size;

	FuncDecl* func = &decl->function;
	String main = String_literal("main");
	if (String_equalsString(func->name, main))  //check special function 'main' signature
	{
		if (!decl->exported)
			error(&decl->location, "function 'main' must exported");

		if (decl->function.resType.id != TYPE_INT)
			error(&decl->location, "function 'main' must return int");
	}

	for (int i = 0; i < func->parameters.size; ++i)
	{
		Parameter* param = Vector_get(&func->parameters, i);
		Variant v;
		v.type = param->type;  //TODO: check type realy existed.
		v.name = param->name;
		v.level = anly->level;
		Stack_push(&anly->stack, &v);
	}

	int hasReturn = 0;
	for (int i = 0; i < func->block.size; ++i)
	{
		Statement* stat = (Statement*)Vector_get(&func->block, i);
		bool has = _Analyzer_statement(anly, decl, stat);
		hasReturn += has;
	}

	if (!hasReturn && func->resType.id != TYPE_VOID)  //check has or hasn't return statement
		error(&decl->location, "function return type not 'void', missing return statement");

	Vector_resize(&anly->stack, lastSize);
	anly->level--;
}

bool _Analyzer_statement(Analyzer* anly, Declaration* decl, Statement* stat)
{
	bool hasReturn = false;
	FuncDecl* func = &decl->function;

	switch (stat->type)
	{
	case STATEMENT_TYPE_DECLARATION:
		if (stat->declaration.type != DECL_TYPE_VARIANT)
			error(&stat->declaration.location, "local declaration must be variant");
		if (stat->declaration.exported)
			error(&stat->declaration.location, "local declaration cannot export");

		_Analyzer_variant(anly, &stat->declaration);
		break;

	case STATEMENT_TYPE_IF:
		hasReturn = _Analyzer_ifStatement(anly,decl, stat);
		break;

	case STATEMENT_TYPE_COMPOUND:
		hasReturn = _Analyzer_compoundStatement(anly, decl, stat);
		break;

	case STATEMENT_TYPE_ASSIGN:
	case STATEMENT_TYPE_ADD_ASSIGN:
	case STATEMENT_TYPE_SUB_ASSIGN:
	case STATEMENT_TYPE_MUL_ASSIGN:
	case STATEMENT_TYPE_DIV_ASSIGN:
	case STATEMENT_TYPE_MOD_ASSIGN:
		_Analyzer_assignStatement(anly, stat);
		break;

	case STATEMENT_TYPE_INC:
	case STATEMENT_TYPE_DEC:
		_Analyzer_incDecStatement(anly, stat);
		break;

	case STATEMENT_TYPE_EXPRESSION:
		_Analyzer_expression(anly, stat->expr);
		break;

	case STATEMENT_TYPE_RETURN:
		_Analyzer_returnStatement(anly, decl, stat);
		hasReturn = true;  //TODO: flow analysis to check has return or not
		break;
	}

	return hasReturn;
}

bool _Analyzer_ifStatement(Analyzer* anly, Declaration* decl, Statement* stat)
{
	Type cond = _Analyzer_expression(anly, stat->ifStat.condition);
	if (!_Analyzer_checkTypeConvert(anly, boolType, cond))
		error(&stat->ifStat.condition->location, "if condition expression cannot convert to bool type");

	bool hasReturn = _Analyzer_statement(anly, decl, stat->ifStat.statement);

	if (stat->ifStat.elseStat)
	{
		bool elseHasReturn = _Analyzer_statement(anly, decl, stat->ifStat.elseStat);
		return hasReturn && elseHasReturn;
	}

	return false;
}

void _Analyzer_assignStatement(Analyzer* anly, Statement* stat)
{
	if (!_Analyzer_checkLvalue(anly, stat->assign.leftExpr))
		error(&stat->assign.leftExpr->location, "expected lvalue");

	Type ltype = _Analyzer_expression(anly, stat->assign.leftExpr);
	Type rtype = _Analyzer_expression(anly, stat->assign.rightExpr);

	switch (stat->type)
	{
	case STATEMENT_TYPE_ADD_ASSIGN:
		rtype = _Analyzer_checkTypeOperate(anly, EXPR_TYPE_ADD, &ltype, &rtype, NULL);
		break;

	case STATEMENT_TYPE_SUB_ASSIGN:
		rtype = _Analyzer_checkTypeOperate(anly, EXPR_TYPE_SUB, &ltype, &rtype, NULL);
		break;

	case STATEMENT_TYPE_MUL_ASSIGN:
		rtype = _Analyzer_checkTypeOperate(anly, EXPR_TYPE_MUL, &ltype, &rtype, NULL);
		break;

	case STATEMENT_TYPE_DIV_ASSIGN:
		rtype = _Analyzer_checkTypeOperate(anly, EXPR_TYPE_DIV, &ltype, &rtype, NULL);
		break;

	case STATEMENT_TYPE_MOD_ASSIGN:
		rtype = _Analyzer_checkTypeOperate(anly, EXPR_TYPE_MOD, &ltype, &rtype, NULL);
		break;
	}

	if (rtype.id == TYPE_INIT)
		error(&stat->assign.leftExpr->location, "lvalue and rvalue expression type not support this operator");

	if (!_Analyzer_checkTypeConvert(anly, ltype, rtype))
		error(&stat->location, "rvalue expression type cannot convert to lvalue type");

	if (stat->type == STATEMENT_TYPE_DIV_ASSIGN || stat->type == STATEMENT_TYPE_MOD_ASSIGN)
	{
		if (_Analyzer_checkZero(anly, stat->assign.rightExpr))  //TODO: compile time eval expression value
			error(&stat->assign.rightExpr->location, "division by zero");
	}
}

void _Analyzer_incDecStatement(Analyzer* anly, Statement* stat)
{
	if (!_Analyzer_checkLvalue(anly, stat->incExpr))
		error(&stat->incExpr->location, "expected lvalue");

	Type ltype = _Analyzer_expression(anly, stat->incExpr);

	switch (ltype.id)
	{
	case TYPE_INT:
		break;

	default:
		error(&stat->location, "expression type not support this operator");
	}
}

bool _Analyzer_compoundStatement(Analyzer* anly, Declaration* decl, Statement* stat)
{
	anly->level++;
	int lastSize = anly->stack.size;

	int hasReturn = 0;
	for (int i = 0; i < stat->compound.size; ++i)
	{
		Statement* subStat = Vector_get(&stat->compound, i);
		bool has = _Analyzer_statement(anly, decl, subStat);
		hasReturn += has;
	}

	Vector_resize(&anly->stack, lastSize);
	anly->level--;

	return hasReturn != 0;
}

void _Analyzer_returnStatement(Analyzer* anly, Declaration* decl, Statement* stat)
{
	if (stat->returnExpr && decl->function.resType.id == TYPE_VOID)
		error(&stat->location, "function return type is 'void', cannot return value");

	if (!stat->returnExpr && decl->function.resType.id != TYPE_VOID)
		error(&stat->location, "function return type not 'void', return statement must with expression");

	if (stat->returnExpr)
	{
		Type type = _Analyzer_expression(anly, stat->returnExpr);
		if (!_Analyzer_checkTypeConvert(anly, decl->function.resType, type))
			error(&stat->returnExpr->location, "return expression type cannot convert to function result");
	}
}

Type _Analyzer_expression(Analyzer* anly, Expression* expr)
{
	Variant* var = NULL;
	Declaration* decl = NULL;

	switch (expr->type)
	{
	case EXPR_TYPE_IDENT:
		var = _Analyzer_findVariant(anly, expr->identExpr);
		if (!var)
		{
			error(&expr->location, "undefined variant '" String_FMT "'", String_arg(expr->identExpr));
			return errorType;
		}
		return var->type;

	case EXPR_TYPE_INT:
		return intType;

	case EXPR_TYPE_BOOL:
		return boolType;

	case EXPR_TYPE_CALL:
		return _Analyzer_callExpression(anly, expr);

	case EXPR_TYPE_PLUS:
	case EXPR_TYPE_MINUS:
	case EXPR_TYPE_NOT:
		return _Analyzer_unaryExpression(anly, expr);

	case EXPR_TYPE_ADD:
	case EXPR_TYPE_SUB:
	case EXPR_TYPE_MUL:
	case EXPR_TYPE_DIV:
	case EXPR_TYPE_MOD:
	case EXPR_TYPE_NE:
	case EXPR_TYPE_EQ:
		return _Analyzer_binaryExpression(anly, expr);

	default:
		error(&expr->location, "incorrect expression");
	}
	return errorType;
}

Type _Analyzer_callExpression(Analyzer* anly, Expression* expr)
{
	if (expr->callExpr.func->type != EXPR_TYPE_IDENT)
		error(&expr->callExpr.func->location, "invalid function identifier");

	Declaration* decl = _Analyzer_findFunction(anly, expr->callExpr.func->identExpr);  //check the function exists
	if (!decl)
	{
		error(&expr->location, "undefined function '" String_FMT "'", String_arg(expr->callExpr.func->identExpr));
		return errorType;
	}

	if (expr->callExpr.args.size != decl->function.parameters.size)  //TODO: support variadic parameter
	{
		error(&expr->location, "function argument count not match");  //TODO: show difference
		return errorType;
	}

	for (int i = 0; i < expr->callExpr.args.size; ++i)
	{
		Parameter* param = Vector_get(&decl->function.parameters, i);
		Expression* arg = Vector_get(&expr->callExpr.args, i);
		Type type = _Analyzer_expression(anly, arg);
		if (type.id != param->type.id)  //TODO: support implict type cast
			error(&arg->location, "function argument type not match");  //TODO: show details
	}

	return decl->function.resType;
}

Type _Analyzer_unaryExpression(Analyzer* anly, Expression* expr)
{
	Type rtype = _Analyzer_expression(anly, expr->unaryExpr);

	switch (expr->type)
	{
	case EXPR_TYPE_PLUS:
	case EXPR_TYPE_MINUS:
	case EXPR_TYPE_NOT:
		//TODO: check variant used is inited
		rtype = _Analyzer_checkTypeOperate(anly, expr->type, &rtype, NULL, NULL);
		if (rtype.id == TYPE_INIT)
			error(&expr->unaryExpr->location, "expression type not support this operator");

		break;

	default:
		error(&expr->location, "unsupported unary operator");
	}

	return rtype;
}

Type _Analyzer_binaryExpression(Analyzer* anly, Expression* expr)
{
	Type ltype = _Analyzer_expression(anly, expr->binaryExpr.leftExpr);
	Type rtype = _Analyzer_expression(anly, expr->binaryExpr.rightExpr);

	ltype = _Analyzer_checkTypeOperate(anly, expr->type, &ltype, &rtype, NULL);  //TODO: show detail
	if (ltype.id == TYPE_INIT)
		error(&expr->binaryExpr.leftExpr->location, "expression type not support this operator");

	if (expr->type == EXPR_TYPE_DIV || expr->type == EXPR_TYPE_MOD)
	{
		if (_Analyzer_checkZero(anly, expr->binaryExpr.rightExpr))  //TODO: compile time eval expression value
			error(&expr->binaryExpr.rightExpr->location, "division by zero");
	}

	return ltype;
}

bool _Analyzer_checkTypeConvert(Analyzer* anly, Type left, Type right)
{
	if (left.id == right.id)  //TODO: more type convert regular
		return true;
	return false;
}

Type _Analyzer_checkTypeOperate(Analyzer* anly, ExprType exprType, Type* t1, Type* t2, Type* t3)
{
	switch (exprType)
	{
	case EXPR_TYPE_PLUS:  //unary
	case EXPR_TYPE_MINUS:
		switch (t1->id)
		{
		case TYPE_INT:
			return *t1;

		default:
			return errorType;
		}
		//break
	case EXPR_TYPE_NOT:
		if (t1->id == TYPE_BOOL)
			return *t1;
		return errorType;
		
	case EXPR_TYPE_ADD:  //binary
	case EXPR_TYPE_SUB:
	case EXPR_TYPE_MUL:
	case EXPR_TYPE_DIV:
	case EXPR_TYPE_MOD:
		switch (t1->id)
		{
		case TYPE_INT:
			switch (t2->id)
			{
			case TYPE_INT:
				return *t1;

			default:
				return errorType;
			}
			break;
		}

	case EXPR_TYPE_NE:
	case EXPR_TYPE_EQ:
		switch (t1->id)
		{
		case TYPE_INT:
			switch (t2->id)
			{
			case TYPE_INT:
				return boolType;

			default:
				return errorType;
			}
			break;

		case TYPE_BOOL:
			switch (t2->id)
			{
			case TYPE_BOOL:
				return boolType;

			default:
				return errorType;
			}
		}
	}

	return errorType;
}

bool _Analyzer_checkLvalue(Analyzer* anly, Expression* expr)
{
	if (expr->type == EXPR_TYPE_IDENT)  //TODO: more type regular
		return true;
	return false;
}

bool _Analyzer_checkZero(Analyzer* anly, Expression* expr)
{
	switch (expr->type)  //TODO: compile time eval expression value
	{
	case EXPR_TYPE_INT:
		return expr->intExpr == 0;

	case EXPR_TYPE_PLUS:
	case EXPR_TYPE_MINUS:
		return _Analyzer_checkZero(anly,  expr->unaryExpr);
	}

	return false;
}

Variant* _Analyzer_findVariant(Analyzer* anly, String name)
{
	for (int i = anly->stack.size - 1; i >= 0; --i)
	{
		Variant* v = Vector_get(&anly->stack, i);
		if (v->level <= anly->level && String_equalsString(v->name, name))
			return v;
	}
	return NULL;
}

Declaration* _Analyzer_findFunction(Analyzer* anly, String name)
{
	for (int i = 0; i < anly->module->functions.size; ++i)
	{
		Declaration* decl = Vector_get(&anly->module->functions, i);
		if (String_equalsString(name, decl->function.name))
			return decl;
	}
	return NULL;
}
