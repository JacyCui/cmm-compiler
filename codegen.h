#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>

extern const char* asmHeader;

void translateIRToCode(FILE *fd);

#endif
