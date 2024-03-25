#include <stdlib.h>
#include <string.h>

#include "analyzer.h"
#include "message.h"

struct Variant
{
	Type type;
	String name;
	int level;
};
typedef struct Variant Variant;

static void Variant_init(Variant* v)
{
	memset(v, 0, sizeof(Variant));
}

static void _Analyzer_variant(Analyzer* anly, Declaration* decl);
static void _Analyzer_function(Analyzer* anly, Declaration* decl);
static bool _Analyzer_statement(Analyzer* anly, Declaration* decl, Statement* stat);
static void _Analyzer_assignStatement(Analyzer* anly, Statement* stat);
static bool _Analyzer_compoundStatement(Analyzer* anly, Declaration* decl, Statement* stat);
static Type _Analyzer_expression(Analyzer* anly, Expression* exr);
static Type _Analyzer_callExpression(Analyzer* anly, Expression* expr);
static Type _Analyzer_unaryExpression(Analyzer* anly, Expression* expr);
static Type _Analyzer_binaryExpression(Analyzer* anly, Expression* expr);
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
		if (v->level == anly->level && String_compare(&v->name, decl->variant.name.data) == 0)
			error(&decl->location, "variant '" String_FMT "' duplicate", String_arg(decl->name));  //TODO: show existed variant line
	}

	Variant variant;
	
	switch (decl->variant.type.id)
	{
	case TYPE_INT:
		variant.type = decl->variant.type;
		variant.name = decl->variant.name;
		variant.level = anly->level;
		if (decl->variant.initExpr)
		{
			Type type = _Analyzer_expression(anly, decl->variant.initExpr);
			if (type.id != variant.type.id)
				error(&decl->variant.initExpr->location, "variant init expression type not match");
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
	if (String_compare(&func->name, "main") == 0)  //check special function 'main' signature
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

	case STATEMENT_TYPE_ASSIGN:
		_Analyzer_assignStatement(anly, stat);
		break;

	case STATEMENT_TYPE_COMPOUND:
		hasReturn = _Analyzer_compoundStatement(anly, decl, stat);
		break;

	case STATEMENT_TYPE_EXPRESSION:
		_Analyzer_expression(anly, stat->expr);
		break;

	case STATEMENT_TYPE_RETURN:
		//return semantic check
		if (stat->returnExpr && func->resType.id == TYPE_VOID)
			error(&stat->location, "function return type is 'void', cannot return value");
		if (!stat->returnExpr && func->resType.id != TYPE_VOID)
			error(&stat->location, "function return type not 'void', return statement must with expression");

		//return type check
		if (stat->returnExpr)
		{
			Type type = _Analyzer_expression(anly, stat->returnExpr);
			if (type.id != func->resType.id)  //TODO: support implict type cast
				error(&stat->location, "function return type missmatch");
		}

		hasReturn = true;
		break;
	}

	return hasReturn;
}

void _Analyzer_assignStatement(Analyzer* anly, Statement* stat)
{
	if (!_Analyzer_checkLvalue(anly, stat->assign.leftExpr))
		error(&stat->assign.leftExpr->location, "expected lvalue");

	Type ltype = _Analyzer_expression(anly, stat->assign.leftExpr);
	Type rtype = _Analyzer_expression(anly, stat->assign.rightExpr);

	if (ltype.id != rtype.id)  //TODO: implict type cast, more type regular
		error(&stat->location, "lvalue and rvalue expression type not match");
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

	case EXPR_TYPE_CALL:
		return _Analyzer_callExpression(anly, expr);

	case EXPR_TYPE_PLUS:
	case EXPR_TYPE_MINUS:
		return _Analyzer_unaryExpression(anly, expr);

	case EXPR_TYPE_ADD:
	case EXPR_TYPE_SUB:
	case EXPR_TYPE_MUL:
	case EXPR_TYPE_DIV:
		return _Analyzer_binaryExpression(anly, expr);
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
		switch (rtype.id)
		{
		case TYPE_INT:
			break;
		default:
			error(&expr->unaryExpr->location, "expression type not support unary add operator");
		}

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

	switch (expr->type)
	{
	case EXPR_TYPE_ADD:
	case EXPR_TYPE_SUB:
	case EXPR_TYPE_MUL:
	case EXPR_TYPE_DIV:
		//TODO: more operation type regular
		switch (ltype.id)
		{
		case TYPE_INT:
			break;
		default:
			error(&expr->binaryExpr.leftExpr->location, "expression type not support binary add operator");
		}

		if (ltype.id != rtype.id)  //TODO: implict type cast, more type regular
			error(&expr->location, "left and right expression type not match");

		break;

	default:
		error(&expr->location, "sunsupported binary operator");
	}

	if (expr->type == EXPR_TYPE_DIV && _Analyzer_checkZero(anly, expr->binaryExpr.rightExpr))
		error(&expr->binaryExpr.rightExpr->location, "division by zero");

	return ltype;  //TODO: more type cast regular
}

bool _Analyzer_checkLvalue(Analyzer* anly, Expression* expr)
{
	if (expr->type == EXPR_TYPE_IDENT)  //TODO: more type regular
		return true;
	return false;
}

bool _Analyzer_checkZero(Analyzer* anly, Expression* expr)
{
	switch (expr->type)
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
		if (v->level <= anly->level && String_compare(&v->name, name.data) == 0)
			return v;
	}
	return NULL;
}

Declaration* _Analyzer_findFunction(Analyzer* anly, String name)
{
	for (int i = 0; i < anly->module->functions.size; ++i)
	{
		Declaration* decl = Vector_get(&anly->module->functions, i);
		if (String_compare(&name, decl->function.name.data) == 0)
			return decl;
	}
	return NULL;
}
