#ifndef CLAM_TYPE_H
#define CLAM_TYPE_H

#include "token.h"

struct Type
{
	String name;
	TokenValue value;
};
typedef struct Type Type;

#endif