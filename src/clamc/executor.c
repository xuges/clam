#include <stdio.h>
#include <string.h>

#include "executor.h"
#include "message.h"

struct Value
{
	String name;
	Type type;
	int level;
	union
	{
		int intValue;
	};
};
typedef struct Value Value;

static void Value_init(Value* v)
{
	String_init(&v->name);
	Type_init(&v->type);
	v->intValue = 0;
}

enum ExecuteResult
{
	EXEC_RESULT_NORMAL,
	EXEC_RESULT_RETURN,
};
typedef enum ExecuteResult ExecuteResult;

static void _Executor_variant(Executor* exec, Declaration* decl);
static void _Executor_function(Executor* exec, Declaration* decl, Vector args);
static ExecuteResult _Executor_statement(Executor* exec, Declaration* decl, Statement* stat);
static ExecuteResult _Executor_compoundStatement(Executor* exec, Declaration* decl,  Statement* stat);
static void _Executor_expression(Executor* exec, Expression* expr);
static void _Executor_callExpression(Executor* exec, Expression* expr);
static void _Executor_assignExpression(Executor* exec, Expression* expr);
static Value* _Executor_findVariant(Executor* exec, String name);
static Declaration* _Executor_fincFunction(Executor* exec, String name);
static void _Executor_enterBlock(Executor* exec);
static void _Executor_leaveBlock(Executor* exec);
static Stack _Executor_enterFunction(Executor* exec, int argc);
static void _Executor_leaveFunction(Executor* exec, Stack base);

void Executor_init(Executor* exec)
{
	Vector_init(&exec->global, sizeof(Value));
	Stack_init(&exec->stack, sizeof(Value));
	Stack_reserve(&exec->stack, 1 * 1024 * 1024);  //1M * sizeof(Value)
	exec->module = 0;
	exec->level = 0;
}

void Executor_destroy(Executor* exec)
{
	Stack_destroy(&exec->stack);
}

void Executor_run(Executor* exec, Module* module)
{
	exec->module = module;
	Vector_resize(&exec->stack, 0);

	//global variants
	for (int i = 0; i < module->declarations.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->declarations, i);
		if (decl->type == DECL_TYPE_VARIANT)
			_Executor_variant(exec, decl);
	}

	//find main
	String main = String_literal("main");
	Declaration* decl = _Executor_fincFunction(exec, main);
	if (!decl)
		error(NULL, 0, 0, "function 'main' not found");

	//call main
	Vector args;
	Vector_init(&args, 0);  //TODO: add main argument
	_Executor_function(exec, decl, args);

	//output main return
	Value* ret = Stack_top(&exec->stack);
	printf("main return %d\n", ret->intValue);
}

void _Executor_variant(Executor* exec, Declaration* decl)
{
	switch (decl->variant.type.id)
	{
	case TYPE_INT:
		if (decl->variant.initExpr)
		{
			_Executor_expression(exec, decl->variant.initExpr);
			Value* exprValue = Stack_top(&exec->stack);
			exprValue->name = decl->variant.name;
			exprValue->level = exec->level;
		}
		break;
	}
	
	if (exec->level == 0)
		Vector_add(&exec->global, Stack_pop(&exec->stack));
}

void _Executor_function(Executor* exec, Declaration* decl, Vector args)
{
	//push arguments
	for (int i = args.size - 1; i >= 0; --i)  //right to left
	{
		Parameter* param = Vector_get(&decl->function.parameters, i);
		Expression* arg = Vector_get(&args, i);
		_Executor_expression(exec, arg);
		Value* v = Stack_top(&exec->stack);
		v->name = param->name;
	}

	Stack base = _Executor_enterFunction(exec, args.size);

	ExecuteResult result = EXEC_RESULT_NORMAL;
	FuncDecl* func = &decl->function;

	for (int i = 0; i < func->block.size; i++)
	{
		Statement* stat = (Statement*)Vector_get(&func->block, i);
		result = _Executor_statement(exec, decl, stat);
		if (result == EXEC_RESULT_RETURN)
			break;
	}

	Value* ret = Stack_top(&exec->stack);

	_Executor_leaveFunction(exec, base);

	//return value
	if (decl->function.resType.id != TYPE_VOID)
		Stack_push(&exec->stack, ret);
}

ExecuteResult _Executor_statement(Executor* exec, Declaration* decl, Statement* stat)
{
	ExecuteResult result = EXEC_RESULT_NORMAL;
	FuncDecl* func = &decl->function;

	switch (stat->type)
	{
	case STATEMENT_TYPE_COMPOUND:
		result = _Executor_compoundStatement(exec, decl, stat);
		break;
		
	case STATEMENT_TYPE_DECLARATION:
		_Executor_variant(exec, &stat->declaration);
		break;

	case STATEMENT_TYPE_EXPRESSION:
		_Executor_expression(exec, stat->expr);
		break;

	case STATEMENT_TYPE_RETURN:
		if (stat->returnExpr)
			_Executor_expression(exec, stat->returnExpr);  //eval expr
		return EXEC_RESULT_RETURN;
	}

	return result;
}

ExecuteResult _Executor_compoundStatement(Executor* exec, Declaration* decl, Statement* stat)
{
	_Executor_enterBlock(exec);

	ExecuteResult result = EXEC_RESULT_NORMAL;

	for (int i = 0; i < stat->compound.size; i++)
	{
		Statement* subStat = (Statement*)Vector_get(&stat->compound, i);
		result = _Executor_statement(exec, decl, subStat);
		if (result == EXEC_RESULT_RETURN)
			break;
	}

	_Executor_leaveBlock(exec);
	return result;
}

void _Executor_expression(Executor* exec, Expression* expr)
{
	//TODO: add more expressions
	Value value;

	switch (expr->type)
	{
	case EXPR_TYPE_IDENT:
		value = *_Executor_findVariant(exec, expr->identExpr);
		value.level = exec->level;
		Stack_push(&exec->stack, &value);
		break;

	case EXPR_TYPE_INT:
		Value_init(&value);
		value.type = intType;
		value.intValue = expr->intExpr;
		value.level = exec->level;
		Stack_push(&exec->stack, &value);
		break;

	case EXPR_TYPE_CALL:
		_Executor_callExpression(exec, expr);
		break;

	case EXPR_TYPE_ASSIGN:
		_Executor_assignExpression(exec, expr);
		break;
	}
}

void _Executor_callExpression(Executor* exec, Expression* expr)
{
	//find function
	Declaration* decl = _Executor_fincFunction(exec, expr->callExpr.func->identExpr);

	//call function
	_Executor_function(exec, decl, expr->callExpr.args);
}

void _Executor_assignExpression(Executor* exec, Expression* expr)
{
	//find lvalue variant
	Value* lvalue = _Executor_findVariant(exec, expr->assignExpr.lvalueExpr->identExpr);  //TODO: process expression first

	//eval rvalue
	_Executor_expression(exec, expr->assignExpr.rvalueExpr);
	Value* rvalue = Stack_pop(&exec->stack);

	//assign
	lvalue->intValue = rvalue->intValue;

	//push
	Stack_push(&exec->stack, lvalue);
}

Value* _Executor_findVariant(Executor* exec, String name)
{
	for (int i = exec->stack.size - 1; i >= 0; --i)  //reverse (local first)
	{
		Value* value = Vector_get(&exec->stack, i);
		if (value->level <= exec->level && String_compare(&value->name, name.data) == 0)
			return value;
	}

	for (int i = 0; i < exec->global.size; ++i)  //global second
	{
		Value* value = Vector_get(&exec->global, i);
		if (String_compare(&value->name, name.data) == 0)
			return value;
	}

	return NULL;
}

Declaration* _Executor_fincFunction(Executor* exec, String name)
{
	for (int i = 0; i < exec->module->functions.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&exec->module->functions, i);
		if (String_compare(&name, decl->function.name.data) == 0)
		{
			//found function
			return decl;
		}
	}
	return NULL;
}

void _Executor_enterBlock(Executor* exec)
{
	exec->level++;
}

void _Executor_leaveBlock(Executor* exec)
{
	exec->level--;
}

Stack _Executor_enterFunction(Executor* exec, int argc)
{
	exec->level++;
	Stack base = exec->stack;  //save stack

	//adjust stack
	int diff = exec->stack.size - argc;
	int offset = diff * exec->stack.elemSize;
	exec->stack.data = (char*)exec->stack.data + offset;
	exec->stack.size = argc;
	exec->stack.cap -= diff;

	return base;
}

void _Executor_leaveFunction(Executor* exec, Stack base)
{
	exec->stack = base;  //restore stack
	exec->level--;
}
