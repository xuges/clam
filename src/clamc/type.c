#include "type.h"

void Type_init(Type* typ)
{
	String_init(&typ->name);
	typ->id = TYPE_INIT;
}

Type errorType =
{
	String_literal("error-type"),
	TYPE_INIT
};

Type voidType =
{
	String_literal("void"),
	TYPE_VOID
};

Type intType =
{
	String_literal("int"),
	TYPE_INT
};


Type boolType =
{
	String_literal("bool"),
	TYPE_BOOL
};