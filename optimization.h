#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include "bb.h"

void flodTemp(bb_t b);

void rmLocalCommonSubexpression(bb_t b);

void rmDeadCode(bb_t b);

void constFolding(bb_t b);

#endif
