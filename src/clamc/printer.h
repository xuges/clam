#ifndef CLAM_PRINTER_H
#define CLAM_PRINTER_H

#include "module.h"
#include "lexer.h"

struct Printer
{
	Module* module;
	int level;
};
typedef struct Printer Printer;

void Printer_init(Printer* p);
void Printer_destroy(Printer* p);

void Printer_printLex(Printer* p, Lexer* lex);
void Printer_printAst(Printer* p, Module* module);

#endif