#ifndef CLAM_SOURCE_H
#define CLAM_SOURCE_H

#include <stdbool.h>

struct Source
{
	const char* name;
	char* data;
	char* end;
	char* iter;
	int size;
	int type;
};

typedef struct Source Source;

//init source from code string
void Source_init(Source* src, char* code);

//open source file
bool Source_open(Source* src, const char* filename);

void Source_destroy(Source* src);

bool Source_isEof(Source* src);
const char* Source_peek(Source* src);
const char* Source_next(Source* src);
void Source_reset(Source* src);

#endif