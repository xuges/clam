#ifndef CLAM_TYPE_H
#define CLAM_TYPE_H

#include "token.h"

enum TypeId
{
	TYPE_VOID,
	TYPE_INT,
};
typedef enum TypeId TypeId;

struct Type
{
	String name;
	TokenValue value;
	TypeId id;
};
typedef struct Type Type;

#endif