#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "source.h"

enum _SoruceType
{
	SOURCE_TYPE_INIT,
	SOURCE_TYPE_STRING,
	SOURCE_TYPE_FILE_DATA,
	//SOURCE_TYPE_FILE_MAPPING,
};
typedef enum _SourceType SourceType;

void Source_init(Source* src, char* code)
{
	src->name = "<string>";
	src->data = code;
	src->size = strlen(code);
	src->end = code + src->size;
	src->iter = src->data;
	src->type = SOURCE_TYPE_STRING;
}

bool Source_open(Source* src, const char* filename)
{
	if (!filename)
		return false;

	//open file
	FILE* f = fopen(filename, "rb");
	if (!f)
		return false;

	//get file size
	int beg = ftell(f);
	fseek(f, 0, SEEK_END);
	int end = ftell(f);
	int size = end - beg;
	fseek(f, beg, SEEK_SET);

	if (!size)
		return false;

	//TODO: mmap large file

	char* buf = (char*)malloc(size + 1);
	if (!buf)
	{
		fclose(f);
		return false;
	}
	buf[size] = 0;    //null terminal for debug

	fread(buf, 1, size, f);
	fclose(f);

	src->name = filename;
	src->data = buf;
	src->end = buf + size;
	src->iter = src->data;
	src->type = SOURCE_TYPE_FILE_DATA;

	return true;
}

void Source_destroy(Source* src)
{
	if (src->type == SOURCE_TYPE_FILE_DATA)
	{
		free(src->data);
		src->name = NULL;
		src->data = NULL;
		src->end = NULL;
		src->iter = NULL;
	}
}

bool Source_isEof(Source* src)
{
	return src->iter >= src->end;
}

const char* Source_peek(Source* src)
{
	return src->iter;
}

const char* Source_next(Source* src)
{
	return src->iter++;
}

void Source_reset(Source* src)
{
	src->iter = src->data;
}