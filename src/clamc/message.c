#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "message.h"

void warning(SourceLocation* loc, const char* fmt, ...)
{
	va_list vp;
	va_start(vp, fmt);
	(loc && loc->filename) ? fprintf(stderr, "%s:%d:%d warnning: ", loc->filename, loc->line, loc->colum) : fprintf(stderr, "clamc: warnning: ");
	vfprintf(stderr, fmt, vp);
	fprintf(stderr, "\n");
	va_end(vp);
}

void error(SourceLocation* loc, const char* fmt, ...)
{
	va_list vp;
	va_start(vp, fmt);
	(loc && loc->filename) ? fprintf(stderr, "%s:%d:%d error: ", loc->filename, loc->line, loc->colum) : fprintf(stderr, "clamc: error: ");
	vfprintf(stderr, fmt, vp);
	fprintf(stderr, "\n");
	va_end(vp);
	exit(-1);
}

void fatal(SourceLocation* loc, const char* fmt, ...)
{
	va_list vp;
	va_start(vp, fmt);
	(loc && loc->filename) ? fprintf(stderr, "%s:%d:%d error: ", loc->filename, loc->line, loc->colum) : fprintf(stderr, "clamc: error: ");
	vfprintf(stderr, fmt, vp);
	fprintf(stderr, "\n");
	va_end(vp);
	exit(-1);
}
