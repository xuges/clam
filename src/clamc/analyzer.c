#include <stdlib.h>
#include <string.h>

#include "analyzer.h"
#include "message.h"

enum AnalysisResult
{
	ANALY_RESULT_NORMAL,
	ANALY_RESULT_RETURN,
};
typedef enum AnalysisResult AnalysisResult;

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
static void _Analyzer_statement(Analyzer* anly, FuncDecl* func, Statement* stat);
static void _Analyzer_expression(Analyzer* anly, Expression* expr);
static TypeId _Analyzer_expressionType(Analyzer* anly, Expression* exr);
static void _Analyzer_callExpression(Analyzer* anly, Expression* expr);
static Variant* _Analyzer_findVariant(Analyzer* anly, String name);
static Declaration* _Analyzer_findFunction(Analyzer* anly, String name);

void Analyzer_init(Analyzer* anly)
{
	Stack_init(&anly->stack, sizeof(Variant));
	anly->level = 0;
	anly->hasReturn = false;
}

void Analyzer_destroy(Analyzer* anly)
{
	Stack_destroy(&anly->stack);
}

//TODO: semantic error not exit
void Analyzer_generate(Analyzer* anly, Module* module, Generator* gen)
{
	//TODO: prepare package name
	anly->module = module;
	anly->gen = gen;

	for (int i = 0; i < module->declarations.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->declarations, i);
		switch (decl->type)
		{
		case DECL_TYPE_VARIANT:
			_Analyzer_variant(anly, decl);
			break;

		case DECL_TYPE_FUNCTION:
			_Analyzer_function(anly, decl);
			break;
		}
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
			TypeId id = _Analyzer_expressionType(anly, decl->variant.initExpr);
			if (id != variant.type.id)
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
	anly->hasReturn = false;

	FuncDecl* func = &decl->function;
	if (String_compare(&func->name, "main") == 0)
	{
		if (!decl->exported)
			error(&decl->location, "function 'main' must exported");

		if (decl->function.resType.id != TYPE_INT)
			error(&decl->location, "function 'main' must return int");
	}

	Generator_enterDeclaration(anly->gen, decl);

	for (int i = 0; i < func->block.size; i++)
	{
		Statement* stat = (Statement*)Vector_get(&func->block, i);
		_Analyzer_statement(anly, func, stat);
	}

	if (!anly->hasReturn && func->resType.id != TYPE_VOID)
		error(&decl->location, "function return type not 'void', must return a value");

	Generator_leaveDeclaration(anly->gen, decl);

	while (anly->stack.size)  //clean variants
	{
		Variant* v = Stack_top(&anly->stack);
		if (v->level != anly->level)
			break;
		Stack_pop(&anly->stack);
	}
	anly->level--;
}

void _Analyzer_statement(Analyzer* anly, FuncDecl* func, Statement* stat)
{
	Generator_enterStatement(anly->gen, stat);

	switch (stat->type)
	{
	case STATEMENT_TYPE_EXPRESSION:
		_Analyzer_expression(anly, stat->expr);

	case STATEMENT_TYPE_RETURN:
		if (stat->returnExpr)
			_Analyzer_expression(anly, stat->returnExpr);
		else if (func->resType.id != TYPE_VOID)
			error(&stat->location, "function return type not 'void', return must with expression");
		anly->hasReturn = true;
		break;
	}

	Generator_leaveStatement(anly->gen, stat);
}

void _Analyzer_expression(Analyzer* anly, Expression* expr)
{
	switch (expr->type)
	{
	case EXPR_TYPE_INT:
		Generator_genPrimaryExpression(anly->gen, expr);
		break;
	case EXPR_TYPE_CALL:
		_Analyzer_callExpression(anly, expr);
		break;
	}
}

TypeId _Analyzer_expressionType(Analyzer* anly, Expression* expr)
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
			return TYPE_VOID;
		}
		return var->type.id;

	case EXPR_TYPE_INT:
		return TYPE_INT;

	case EXPR_TYPE_CALL:
		decl = _Analyzer_findFunction(anly, expr->callExpr.func->identExpr);
		if (!decl)
		{
			error(&expr->location, "undefined function '" String_FMT "'", String_arg(expr->callExpr.func->identExpr));
			return TYPE_VOID;
		}
		return decl->function.resType.id;
	}
	return TYPE_VOID;
}

void _Analyzer_callExpression(Analyzer* anly, Expression* expr)
{
	if (expr->callExpr.func->type != EXPR_TYPE_IDENT)
		error(&expr->callExpr.func->location, "invalid function identifier");

	Expression* func = expr->callExpr.func;

	for (int i = 0; i < anly->module->functions.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&anly->module->functions, i);
		if (String_compare(&func->identExpr, decl->function.name.data) == 0)
		{
			//found function
			Generator_genCallExpression(anly->gen, expr);
			return;
		}
	}

	error(&expr->callExpr.func->location, "undefined function '"String_FMT"'", String_arg(func->identExpr));
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
