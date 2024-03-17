#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "generator.h"
#include "message.h"

void _Generator_indent(Generator* gen, StringBuffer* buf);

void Generator_init(Generator* gen, GenerateTarget target)
{
	//switch (target)
	//{
	//case GENERATE_TARGE_C:
	//	StringBuffer_init(&gen->declarations);
	//	StringBuffer_init(&gen->definitions);
	//	break;
	//
	//case GENERATE_TARGE_C_FILE:
	//	gen->declFile = NULL;
	//	gen->defFile = NULL;
	//	break;
	//}

	StringBuffer_init(&gen->declarations);
	StringBuffer_init(&gen->definitions);
	gen->target = target;
	gen->indentLevel = 0;
}

void Generator_destroy(Generator* gen)
{
	StringBuffer_destroy(&gen->declarations);
	StringBuffer_destroy(&gen->definitions);
	
	//switch (gen->target)
	//{
	//case GENERATE_TARGE_C:
	//	StringBuffer_destroy(&gen->declarations);
	//	StringBuffer_destroy(&gen->definitions);
	//	break;
	//
	//case GENERATE_TARGE_C_FILE:
	//	if (gen->declFile)
	//	{
	//		fclose((FILE*)gen->declFile);
	//		gen->declFile = NULL;
	//	}
	//
	//	if (gen->defFile)
	//	{
	//		fclose((FILE*)gen->defFile);
	//		gen->defFile = NULL;
	//	}
	//	break;
	//}
}

void Generator_enterDeclaration(Generator* gen, Declaration* decl)
{
	gen->indentLevel++;

	if (decl->type == DECL_TYPE_FUNCTION)
	{
		FuncDecl* func = &decl->function;

		if (String_compare(&func->name, "main") != 0)  //not 'main'
		{
			if (!decl->exported)
				StringBuffer_append(&gen->declarations, "static ");

			StringBuffer_appendString(&gen->declarations, &func->resType.name);
			StringBuffer_append(&gen->declarations, " ");

			StringBuffer_appendString(&gen->declarations, &func->name);
			StringBuffer_append(&gen->declarations, "();\n");    //TODO: add parameter list
		}

		StringBuffer_appendString(&gen->definitions, &func->resType.name);
		StringBuffer_append(&gen->definitions, " ");

		StringBuffer_appendString(&gen->definitions,  &func->name);
		StringBuffer_append(&gen->definitions, "() {\n");
	}
}

void Generator_leaveDeclaration(Generator* gen, Declaration* decl)
{
	if (decl->type == DECL_TYPE_FUNCTION)
	{
		gen->indentLevel--;
		_Generator_indent(gen, &gen->definitions);
		StringBuffer_append(&gen->definitions, "}\n");
	}
}

void Generator_enterStatement(Generator* gen, Statement* stat)
{
	switch (stat->type)
	{
	case STATEMENT_TYPE_EXPRESSION:
		
	case STATEMENT_TYPE_RETURN:
		_Generator_indent(gen, &gen->definitions);
		StringBuffer_append(&gen->definitions, "return ");
		return;
	}

}

void Generator_leaveStatement(Generator* gen, Statement* stat)
{
	//TODO: check statement type
	StringBuffer_append(&gen->definitions, ";\n");
}

void Generator_genPrimaryExpression(Generator* gen, Expression* expr)
{
	char buf[64];
	int len;

	switch (expr->type)
	{
	case EXPR_TYPE_INT:
		len = snprintf(buf, sizeof(buf), "%d", expr->intExpr);
		StringBuffer_appendN(&gen->definitions, buf, len);
		break;
	}
}

void Generator_genCallExpression(Generator* gen, Expression* expr)
{
	Expression* func = expr->callExpr.func;
	StringBuffer_appendString(&gen->definitions, &func->identExpr);
	StringBuffer_append(&gen->definitions, "(");
	//TODO: add arguments
	StringBuffer_append(&gen->definitions, ")");
}

void _Generator_indent(Generator* gen, StringBuffer* buf)
{
	for (int i = 0; i < gen->indentLevel; ++i)
		StringBuffer_append(buf, "\t");
}
