#include "analyzer.h"
#include "message.h"

enum AnalysisResult
{
	ANALY_RESULT_NORMAL,
	ANALY_RESULT_RETURN,
	ANALY_RESULT_RETURN_EXPR,
};
typedef enum AnalysisResult AnalysisResult;

static void _Analyzer_function(Analyzer* anly, Declaration* decl);
static AnalysisResult _Analyzer_statement(Analyzer* anly, Statement* stat);
static void _Analyzer_expression(Analyzer* anly, Expression* expr);
static void _Analyzer_callExpression(Analyzer* anly, Expression* expr);

void Analyzer_init(Analyzer* anly)
{

}

void Analyzer_destroy(Analyzer* anly)
{
}

//TODO: semantic error not exit
void Analyzer_generate(Analyzer* anly, Module* module, Generator* gen)
{
	//TODO: prepare package name
	//Generator_prepare(gen, filename);

	anly->module = module;
	anly->gen = gen;

	for (int i = 0; i < module->declarations.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->declarations, i);
		if (decl->type == DECL_TYPE_FUNCTION)
			_Analyzer_function(anly, decl);
	}
}

void _Analyzer_function(Analyzer* anly, Declaration* decl)
{
	FuncDecl* func = &decl->function;
	if (String_compare(&func->name, "main") == 0)
	{
		if (!decl->exported)
			error(&decl->location, "function 'main' must exported");

		if (decl->function.resType.value != TOKEN_VALUE_INT)
			error(&decl->location, "function 'main' must return int");
	}

	Generator_enterDeclaration(anly->gen, decl);

	for (int i = 0; i < func->block.size; i++)
	{
		Statement* stat = (Statement*)Vector_get(&func->block, i);
		AnalysisResult result = _Analyzer_statement(anly, stat);
		switch (result)
		{
		case ANALY_RESULT_RETURN:
			if (func->resType.value)
				error(&stat->location, "function return type not 'void', must return a value");
			goto leave;

		case ANALY_RESULT_RETURN_EXPR:
			goto leave;

		default:
			break;
		}
	}

leave:
	Generator_leaveDeclaration(anly->gen, decl);
}

AnalysisResult _Analyzer_statement(Analyzer* anly, Statement* stat)
{
	Generator_enterStatement(anly->gen, stat);

	AnalysisResult result = ANALY_RESULT_NORMAL;
	switch (stat->type)
	{
	case STATEMENT_TYPE_EXPRESSION:
		_Analyzer_expression(anly, stat->expr);

	case STATEMENT_TYPE_RETURN_EXPR:
		_Analyzer_expression(anly, stat->returnExpr);
		result = ANALY_RESULT_RETURN_EXPR;
		break;

	case STATEMENT_TYPE_RETURN:
		result = ANALY_RESULT_RETURN;
		break;
	}

	Generator_leaveStatement(anly->gen, stat);
	return result;
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
