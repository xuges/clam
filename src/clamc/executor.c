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
	String name;
	ValueType type;
	union
	{
		int intValue;
	};
};
typedef struct Value Value;

static void Value_init(Value* v)
{
	String_init(&v->name);
	v->type = VALUE_TYPE_NULL;
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
static Value _Executor_findVariant(Executor* exec, Expression* identExpr);

void Executor_init(Executor* exec)
{
	Stack_init(&exec->stacks, sizeof(Stack));
	Stack global;
	Stack_init(&global, sizeof(Value));
	Stack_push(&exec->stacks, &global);  //stacks[0] is global stack
}

void Executor_destroy(Executor* exec)
{
	for (int i = 0; i < exec->stacks.size; ++i)
		Stack_destroy(Vector_get(&exec->stacks, i));
	Stack_destroy(&exec->stacks);
}

void Executor_run(Executor* exec, Module* module)
{
	exec->module = module;

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
			if (!decl->exported)
				error(&decl->location, "function 'main' must exported");

			if (decl->function.resType.id != TYPE_INT)
				error(&decl->location, "function 'main' must return int");

			Stack fnStack;
			Stack_init(&fnStack, sizeof(Value));
			Stack_push(&exec->stacks, &fnStack);  //alloc function stack

			_Executor_function(exec, decl);

			Stack* stack = Stack_pop(&exec->stacks);  //free function stack
			Stack_destroy(stack);

			Stack* global = Vector_get(&exec->stacks, 0);  //global stack to return value

			Value ret = *(Value*)Stack_top(Vector_get(&exec->stacks, 0));
			printf("main return %d\n", ret.intValue);
			return;
		}
	}

	error(NULL, 0, 0, "function 'main' not found");
}

void _Executor_variant(Executor* exec, Declaration* decl)
{
	if (decl->variant.type.id == TYPE_VOID)
		error(&decl->location, "variant type cannot be 'void'");

	Stack* stack = Stack_top(&exec->stacks);  //only check current stack
	for (int i = 0; i < stack->size; ++i)
	{
		Value* value = Vector_get(stack, i);
		if (String_compare(&value->name, decl->variant.name.data) == 0)
			error(&decl->location, "variant '" String_FMT "' duplicate", String_arg(decl->name));  //TODO: show existed variant line
	}

	Value value;
	
	switch (decl->variant.type.id)
	{
	case TYPE_INT:  //TODO: abstract types
		value.type = VALUE_TYPE_INT;
		value.name = decl->variant.name;
		value.intValue = 0;
		if (decl->variant.initExpr)
		{
			_Executor_expression(exec, decl->variant.initExpr);  //TODO: direct return
			stack = Stack_top(&exec->stacks);
			Value* exprValue = Stack_pop(stack);
			if (exprValue->type != value.type)
				error(&decl->variant.initExpr->location, "variant init expression type not match");
			value.intValue = exprValue->intValue;
		}
		break;

	default:
		error(&decl->location, "incorrect variant type '" String_FMT "'", String_arg(decl->variant.type.name));
	}
	
	Stack_push(stack, &value);
}

void _Executor_function(Executor* exec, Declaration* decl)
{
	ExecuteResult result = EXEC_RESULT_NORMAL;
	FuncDecl* func = &decl->function;

	for (int i = 0; i < func->block.size; i++)
	{
		Statement* stat = (Statement*)Vector_get(&func->block, i);
		result = _Executor_statement(exec, decl, stat);
		if (result == EXEC_RESULT_RETURN)
			break;
	}

	if (func->resType.id != TYPE_VOID && result != EXEC_RESULT_RETURN)
		error(&decl->location, "function return type not 'void', must return a value");
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
		if (stat->declaration.type != DECL_TYPE_VARIANT)
			error(&stat->declaration.location, "local declaration must be variant");
		if (stat->declaration.exported)
			error(&stat->declaration.location, "local declaration cannot export");

		_Executor_variant(exec, &stat->declaration);
		break;

	case STATEMENT_TYPE_EXPRESSION:
		_Executor_expression(exec, stat->expr);
		break;

	case STATEMENT_TYPE_RETURN:
		if (func->resType.id != TYPE_VOID && stat->returnExpr == NULL)
			error(&stat->location, "function return type not 'void', must return a value");

		if (stat->returnExpr)
		{
			_Executor_expression(exec, stat->returnExpr);  //eval expr

			Stack* stack = Stack_top(&exec->stacks);
			Value* ret = Stack_pop(stack);  //get return value
			Stack* global = Vector_get(&exec->stacks, 0);  //global stack to return value
			Stack_push(global, ret);  //return
		}

		return EXEC_RESULT_RETURN;
	}

	return result;
}

ExecuteResult _Executor_compoundStatement(Executor* exec, Declaration* decl, Statement* stat)
{
	Stack tmp;
	Stack_init(&tmp, sizeof(Value));
	Stack_push(&exec->stacks, &tmp);  //alloc block stack

	ExecuteResult result = EXEC_RESULT_NORMAL;

	for (int i = 0; i < stat->compound.size; i++)
	{
		Statement* subStat = (Statement*)Vector_get(&stat->compound, i);
		result = _Executor_statement(exec, decl, subStat);
		if (result == EXEC_RESULT_RETURN)
			break;
	}

	Stack* stack = Stack_pop(&exec->stacks);  //free block stack
	Stack_destroy(stack);
	return result;
}

void _Executor_expression(Executor* exec, Expression* expr)
{
	//TODO: add more expressions
	Stack* stack = Stack_top(&exec->stacks);
	Value value;

	switch (expr->type)
	{
	case EXPR_TYPE_IDENT:
		value = _Executor_findVariant(exec, expr);
		Stack_push(stack, &value);
		break;

	case EXPR_TYPE_INT:
		Value_init(&value);
		value.type = VALUE_TYPE_INT;
		value.intValue = expr->intExpr;
		Stack_push(stack, &value);
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

			Stack fnStack;
			Stack_init(&fnStack, sizeof(Value));
			Stack_push(&exec->stacks, &fnStack);  //alloc function stack

			_Executor_function(exec, decl);

			Stack* stack = Stack_pop(&exec->stacks);  //free function stack
			Stack_destroy(stack);

			if (decl->function.resType.id != TYPE_VOID)  //has return value
			{
				Stack* global = Vector_get(&exec->stacks, 0);  //global stack to return value
				Value* ret = Stack_pop(global);  //get return value
				Stack_push(Stack_top(&exec->stacks), ret);  //push return value to stack
			}
			
			return;
		}
	}

	error(&expr->callExpr.func->location, "undefined function '"String_FMT"'", String_arg(func->identExpr));
}

Value _Executor_findVariant(Executor* exec, Expression* expr)
{
	Value value;
	Value_init(&value);

	for (int i = exec->stacks.size - 1; i >= 0; --i)  //reverse (local first)
	{
		Stack* stack = Vector_get(&exec->stacks, i);
		for (int j = 0; j < stack->size; ++j)
		{
			Value* value = Vector_get(stack, j);
			if (String_compare(&value->name, expr->identExpr.data) == 0)
				return *value;
		}
	}

	error(&expr->location, "undefined variant '" String_FMT "'", String_arg(expr->identExpr));
	return value;
}
