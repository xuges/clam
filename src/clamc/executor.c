#include <stdio.h>
#include <string.h>

#include "executor.h"
#include "message.h"

enum ValueType
{
	VALUE_TYPE_NULL,
	VALUE_TYPE_INT,
};
typedef enum ValueType ValueType;

struct Value
{
	ValueType type;
	union
	{
		int intValue;
	};
};
typedef struct Value Value;


enum ExecuteResult
{
	EXEC_RESULT_NORMAL,
	EXEC_RESULT_RETURN,
	EXEC_RESULT_RETURN_EXPR,
};
typedef enum ExecuteResult ExecuteResult;

static void _Executor_function(Executor* exec, Declaration* decl);
static ExecuteResult _Executor_statement(Executor* exec, Statement* stat);
static void _Executor_expression(Executor* exec, Expression* expr);
static void _Executor_callExpression(Executor* exec, Expression* callExpr);

void Executor_init(Executor* exec)
{
	Stack_init(&exec->stack, sizeof(Value));
}

void Executor_destroy(Executor* exec)
{
	Stack_destroy(&exec->stack);
}

void Executor_run(Executor* exec, Module* module)
{
	exec->module = module;

	for (int i = 0; i < module->functions.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->functions, i);
		if (String_compare(&decl->function.name, "main") == 0)
		{
			if (!decl->exported)
				error(&decl->location, "function 'main' must exported");

			if (decl->function.resType.value != TOKEN_VALUE_INT)
				error(&decl->location, "function 'main' must return int");

			_Executor_function(exec, decl);

			Value* ret = (Value*)Stack_pop(&exec->stack);
			printf("main return %d\n", ret->intValue);
			return;
		}
	}

	error(NULL, 0, 0, "function 'main' not found");
}

void _Executor_function(Executor* exec, Declaration* decl)
{
	FuncDecl* func = &decl->function;

	for (int i = 0; i < func->block.size; i++)
	{
		Statement* stat = (Statement*)Vector_get(&func->block, i);
		ExecuteResult result = _Executor_statement(exec, stat);
		switch (result)
		{
		case EXEC_RESULT_RETURN:
			if (func->resType.value)
				error(&stat->location, "function return type not 'void', must return a value");
			return;

		case EXEC_RESULT_RETURN_EXPR:
			return;

		default:
			break;
		}
	}
}

ExecuteResult _Executor_statement(Executor* exec, Statement* stat)
{
	Value value;

	switch (stat->type)
	{
	case STATEMENT_TYPE_EXPRESSION:
		_Executor_expression(exec, stat->expr);

	case STATEMENT_TYPE_RETURN_EXPR:
		_Executor_expression(exec, stat->returnExpr);
		return EXEC_RESULT_RETURN_EXPR;

	case STATEMENT_TYPE_RETURN:
		value.type = VALUE_TYPE_NULL;
		Stack_push(&exec->stack, &value);
		return EXEC_RESULT_RETURN;
	}

	return EXEC_RESULT_NORMAL;
}

void _Executor_expression(Executor* exec, Expression* expr)
{
	//TODO: add more expressions
	Value value;
	switch (expr->type)
	{
	case EXPR_TYPE_INT:
		value.type = VALUE_TYPE_INT;
		value.intValue = expr->intExpr;
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
	if (expr->callExpr.func->type != EXPR_TYPE_IDENT)
		error(&expr->callExpr.func->location, "invalid function identifier");

	Expression* func = expr->callExpr.func;

	for (int i = 0; i < exec->module->functions.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&exec->module->functions, i);
		if (String_compare(&func->identExpr, decl->function.name.data) == 0)
		{
			//found function
			_Executor_function(exec, decl);
			return;
		}
	}

	error(&expr->callExpr.func->location, "undefined function '"String_FMT"'", String_arg(func->identExpr));
}
