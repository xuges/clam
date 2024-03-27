#ifndef CLAM_TYPE_H
#define CLAM_TYPE_H

#include "token.h"

enum TypeId
{
	TYPE_INIT,
	TYPE_VOID,
	TYPE_INT,
	TYPE_BOOL,
};
typedef enum TypeId TypeId;

struct Type
{
	String name;
	TypeId id;
};
typedef struct Type Type;

extern Type errorType;
extern Type voidType;
extern Type intType;
extern Type boolType;

void Type_init(Type* typ);

#endif