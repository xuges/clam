#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "strings.h"
#include "message.h"
#include "macro.h"

void String_init(String* s)
{
	s->data = NULL;
	s->length = 0;
}

bool String_equals(String s1, const char* s2)
{
	return String_equalsN(s1, s2, strlen(s2));
}

bool String_equalsN(String s1, const char* s2, int length)
{
	return s1.length == length && strncmp(s1.data, s2, length);
}

bool String_equalsString(String s1, String s2)
{
	return s1.length == s2.length && strncmp(s1.data, s2.data, s1.length) == 0;
}

int String_toInt(String* s, int base)
{
	return strtol(s->data, NULL, base);
}

void StringBuffer_init(StringBuffer* buf)
{
	buf->data = NULL;
	buf->length = 0;
	buf->cap = 0;
}

void StringBuffer_destroy(StringBuffer* buf)
{
	if (buf->data && buf->cap)
	{
		free(buf->data);
		buf->data = NULL;
		buf->length = 0;
		buf->cap = 0;
	}
}

void StringBuffer_resize(StringBuffer* buf, int size)
{
	if (size > buf->cap)
		StringBuffer_reserve(buf, size);
	buf->length = size;
}

void StringBuffer_reserve(StringBuffer* buf, int cap)
{
	if (cap < 1)
		cap = 1;

	if (buf->cap < cap)
	{
		buf->data = realloc(buf->data, cap + 1);
		if (!buf->data)
			fatal(NULL, 0, 0, "out of memory");
		buf->cap = cap + 1;
	}
}

void StringBuffer_reset(StringBuffer* buf)
{
	buf->length = 0;
	if (buf->data)
		buf->data[0] = 0;
}

void StringBuffer_append(StringBuffer* buf, const char* s)
{
	StringBuffer_appendN(buf, s, strlen(s));
}

void StringBuffer_appendN(StringBuffer* buf, const char* s, int length)
{
	if (length > 0)
	{
		int over = buf->length + length;
		if (over > buf->cap)
			StringBuffer_reserve(buf, CLAM_MAX(over, buf->length * 1.5));
		memcpy(buf->data + buf->length, s, length);
		buf->data[over] = 0;
		buf->length = over;
	}
}

void StringBuffer_appendString(StringBuffer* buf, String* s)
{
	StringBuffer_appendN(buf, s->data, s->length);
}

String* StringBuffer_string(StringBuffer* buf)
{
	return (String*)buf;
}

StringBuffer StringBuffer_clone(StringBuffer* buf)
{
	StringBuffer res;
	StringBuffer_init(&res);
	res.data = (char*)malloc(buf->length + 1);
	if (!res.data)
		fatal(NULL, 0, 0, "out of memory");
	memcpy(res.data, buf->data, buf->length);
	res.data[buf->length] = 0;
	res.length = buf->length;
	res.cap = res.length;
	return res;
}

