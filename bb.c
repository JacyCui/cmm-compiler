#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include "bb.h"
#include "config.h"
#include "ir.h"


bbtable_t bbtab[MAX_TABLE_LEN];

int bbtabSize = 0;

static void buildBasicBlocksForFunction(func_t f) {
    static bool isLeader[MAX_TABLE_LEN] = {false};
    isLeader[functab[f].irBegin] = true;
    stmt_t i;
    stmt_t target;
    for (i = functab[f].irBegin + 1; i <= functab[f].irEnd; i++) {
        if (stmtab[i].kind == GOTO || stmtab[i].kind == IFGOTO) {
            target = getLabelStmtByName(stmtab[i].target);
            assert(target != -1);
            isLeader[target] = true;
            isLeader[i + 1] = true;
        }
    }
    bb_t begin = bbtabSize;
    bbtab[bbtabSize].begin = functab[f].irBegin;
    for (i = functab[f].irBegin + 1; i <= functab[f].irEnd; i++) {
        if (isLeader[i]) {
            bbtab[bbtabSize++].end = i - 1;
            bbtab[bbtabSize].begin = i;
        }
    }
    bbtab[bbtabSize++].end = functab[f].irEnd;
    functab[f].firstBB = begin;
    functab[f].lastBB = bbtabSize - 1;
}

void buildBasicBlocks() {
    func_t f;
    for (f = 0; f < functabSize; f++) {
        buildBasicBlocksForFunction(f);
    }
}

void dumpBB(FILE *fd, bb_t b) {
    stmt_t i;
    for (i = bbtab[b].begin; i <= bbtab[b].end; i++) {
        dumpStmt(fd, i);
    }
}

void dumpBasicBlocks(FILE *fd) {
    func_t f;
    int i = 0;
    operand_t j;
    bb_t b;
    for (f = 0; f < functabSize; f++) {
        fprintf(fd, "FUNCTION %s :\n", symtab[functab[f].funcName]);
        for (j = 0; j < functab[f].paramNum; j++) {
            assert(oprtab[functab[f].params[j]].kind == VAR);
            fprintf(fd, "PARAM ");
            dumpOpr(fd, functab[f].params[j]);
            fprintf(fd, "\n");
        }
        for (b = functab[f].firstBB; b <= functab[f].lastBB; b++) {
            fprintf(fd, "[%d] :\n", i++);
            dumpBB(fd, b);
        }
    }
}
