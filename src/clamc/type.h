#ifndef CLAM_TYPE_H
#define CLAM_TYPE_H

#include "token.h"

enum TypeId
{
	TYPE_INIT,
	TYPE_VOID,
	TYPE_INT,
};
typedef enum TypeId TypeId;

struct Type
{
	String name;
	TypeId id;
};
typedef struct Type Type;

extern Type errorType;
extern Type intType;
extern Type voidType;

void Type_init(Type* typ);

#endif