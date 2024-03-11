#ifndef CLAM_STRING_H
#define CLAM_STRING_H

struct String
{
	const char* data;
	int length;
};
typedef struct String String;

void String_init(String* s);
int String_compare(String* s1, const char* s2);
int String_toInt(String* s, int base);

#define String_literal(s) { (s), sizeof((s)) - 1 }
#define String_FMT "%.*s"
#define String_arg(s) (s).length, (s).data

struct StringBuffer
{
	char* data;
	int length;
	int cap;
};
typedef struct StringBuffer StringBuffer;

void StringBuffer_init(StringBuffer* buf);
void StringBuffer_destroy(StringBuffer* buf);

void StringBuffer_resize(StringBuffer* buf, int size);
void StringBuffer_reserve(StringBuffer* buf, int cap);
void StringBuffer_reset(StringBuffer* buf);

void StringBuffer_append(StringBuffer* buf, const char* s);
void StringBuffer_appendN(StringBuffer* buf, const char* s, int length);
void StringBuffer_appendString(StringBuffer* buf, String* s);

String* StringBuffer_string(StringBuffer* buf);

StringBuffer StringBuffer_clone(StringBuffer* buf);

#endif