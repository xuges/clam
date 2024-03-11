#ifndef CLAM_SOURCE_LOCATION_H
#define CLAM_SOURCE_LOCATION_H

struct SourceLocation
{
	const char* filename;
	int line;
	int colum;
};
typedef struct SourceLocation SourceLocation;

#endif