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
static void _Executor_function(Executor* exec, Declaration* decl);
static ExecuteResult _Executor_statement(Executor* exec, Declaration* decl, Statement* stat);
static ExecuteResult _Executor_compoundStatement(Executor* exec, Declaration* decl,  Statement* stat);
static void _Executor_expression(Executor* exec, Expression* expr);
static void _Executor_callExpression(Executor* exec, Expression* callExpr);
static Value* _Executor_findVariant(Executor* exec, Expression* identExpr);

void Executor_init(Executor* exec)
{
	Stack_init(&exec->stack, sizeof(Value));
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

	for (int i = 0; i < module->declarations.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->declarations, i);
		if (decl->type == DECL_TYPE_VARIANT)
			_Executor_variant(exec, decl);
	}

	for (int i = 0; i < module->functions.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->functions, i);
		if (String_compare(&decl->function.name, "main") == 0)
		{
			_Executor_function(exec, decl);  //TODO: main argument
			Value* ret = Stack_pop(&exec->stack);
			printf("main return %d\n", ret->intValue);
			return;
		}
	}

	error(NULL, 0, 0, "function 'main' not found");
}

void _Executor_variant(Executor* exec, Declaration* decl)
{
	Value value;
	value.type = decl->variant.type;
	value.name = decl->variant.name;
	value.level = exec->level;

	switch (decl->variant.type.id)
	{
	case TYPE_INT:
		if (decl->variant.initExpr)
		{
			_Executor_expression(exec, decl->variant.initExpr);
			Value* exprValue = Stack_pop(&exec->stack);
			value.intValue = exprValue->intValue;
		}
		break;
	}
	
	Stack_push(&exec->stack, &value);
}

void _Executor_function(Executor* exec, Declaration* decl)
{
	exec->level++;

	ExecuteResult result = EXEC_RESULT_NORMAL;
	FuncDecl* func = &decl->function;

	for (int i = 0; i < func->block.size; i++)
	{
		Statement* stat = (Statement*)Vector_get(&func->block, i);
		result = _Executor_statement(exec, decl, stat);
		if (result == EXEC_RESULT_RETURN)
			break;
	}

	exec->level--;
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
	exec->level++;

	ExecuteResult result = EXEC_RESULT_NORMAL;

	for (int i = 0; i < stat->compound.size; i++)
	{
		Statement* subStat = (Statement*)Vector_get(&stat->compound, i);
		result = _Executor_statement(exec, decl, subStat);
		if (result == EXEC_RESULT_RETURN)
			break;
	}

	exec->level--;
	return result;
}

void _Executor_expression(Executor* exec, Expression* expr)
{
	//TODO: add more expressions
	Value value;

	switch (expr->type)
	{
	case EXPR_TYPE_IDENT:
		value = *_Executor_findVariant(exec, expr);
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
	}
}

void _Executor_callExpression(Executor* exec, Expression* expr)
{
	//find function
	Expression* func = expr->callExpr.func;

	for (int i = 0; i < exec->module->functions.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&exec->module->functions, i);
		if (String_compare(&func->identExpr, decl->function.name.data) == 0)
		{
			//found function
			int lastSize = exec->stack.size;  //save stack
			_Executor_function(exec, decl);
			if (decl->function.resType.id != TYPE_VOID)  //return value
				lastSize++;
			Vector_resize(&exec->stack, lastSize);  //restore stack
			return;
		}
	}
}

Value* _Executor_findVariant(Executor* exec, Expression* expr)
{
	for (int i = exec->stack.size - 1; i >= 0; --i)  //reverse (local first)
	{
		Value* value = Vector_get(&exec->stack, i);
		if (value->level <= exec->level && String_compare(&value->name, expr->identExpr.data) == 0)
			return value;
	}
	return NULL;
}
