#ifndef CLAM_MESSAGE_H
#define CLAM_MESSAGE_H

#include "source_location.h"

void warning(SourceLocation* location, const char* fmt, ...);
void error(SourceLocation* location, const char* fmt, ...);
void fatal(SourceLocation* location, const char* fmt, ...);

#endif