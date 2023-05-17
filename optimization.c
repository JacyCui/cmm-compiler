#include <stdbool.h>

#include "ir.h"
#include "optimization.h"

void flodTemp(bb_t b) {
    if (bbtabSize != 1) {
        return;
    }
    stmt_t i, j;
    int k;
    bool alive;
    for (i = bbtab[b].begin; i < bbtab[b].end; i++) {
        switch (stmtab[i].kind) {
            case ADD:
            case SUB:
            case MUL:
            case DIV:
            case ASSIGN:
                if (stmtab[i + 1].kind == RETURN && stmtab[i + 1].ret == stmtab[i].res) {
                    stmtab[i + 1].ret = stmtab[i].arg;
                    stmtab[i].kind = NOP;
                    break;
                }
            case READ:
                if (stmtab[i + 1].kind == ASSIGN && stmtab[i + 1].arg == stmtab[i].res) {
                    alive = false;
                    for (j = i + 2; j <= bbtab[b].end; j++) {
                        switch (stmtab[j].kind) {
                            case ADD:
                            case SUB:
                            case MUL:
                            case DIV:
                                if (stmtab[j].arg1 == stmtab[i].res || stmtab[j].arg2 == stmtab[i].res) {
                                    alive = true;
                                }
                                break;
                            case ASSIGN:
                                if (stmtab[j].arg == stmtab[i].res) {
                                    alive = true;
                                }
                                break;
                            case RETURN:
                                if (stmtab[j].ret == stmtab[i].res) {
                                    alive = true;
                                }
                                break;
                            case WRITE:
                                if (stmtab[j].arg == stmtab[i].res) {
                                    alive = true;
                                }
                                break;
                            case CALL:
                                for (k = 0; k < stmtab[j].argsNum; k++) {
                                    if (stmtab[j].args[k] == stmtab[i].res) {
                                        alive = true;
                                    }
                                }
                                break;
                            default: break;
                        }
                    }
                    if (!alive) {
                        stmtab[i].res = stmtab[i + 1].res;
                        stmtab[i + 1].kind = NOP;
                    }
                }
                break;
            default: break;
        }
    }
}

void rmLocalCommonSubexpression(bb_t b) {
    stmt_t i, j;
    ref_t lhs;
    for (i = bbtab[b].begin; i <= bbtab[b].end; i++) {
        switch (stmtab[i].kind) {
            case ADD:
            case SUB:
            case MUL:
            case DIV:
                lhs = -1;
                for (j = bbtab[b].begin; j < i; j++) {
                    switch (stmtab[j].kind) {
                        case ADD:
                        case SUB:
                        case MUL:
                        case DIV:
                            if (stmtab[j].kind == stmtab[i].kind 
                                && stmtab[j].arg1 == stmtab[i].arg1 && stmtab[j].arg2 == stmtab[i].arg2) {
                                lhs = stmtab[j].res;
                            }
                        case ASSIGN:
                            if (stmtab[j].res == stmtab[i].arg1 || stmtab[j].res == stmtab[i].arg2) {
                                lhs = -1;
                            }
                            break;
                        case READ:
                            if (stmtab[j].arg == stmtab[i].arg1 || stmtab[j].arg == stmtab[i].arg2) {
                                lhs = -1;
                            }
                            break;
                        default: break;
                    }
                }
                if (lhs != -1) {
                    stmtab[i].kind = ASSIGN;
                    stmtab[i].arg = lhs;
                }
                break;
            default: break;
        }
    }
}

void rmDeadCode(bb_t b) {
    if (bbtabSize != 1) {
        return;
    }
    stmt_t i, j;
    int k;
    bool alive;
    for (i = bbtab[b].begin; i <= bbtab[b].end; i++) {
        switch (stmtab[i].kind) {
            case ADD:
            case SUB:
            case MUL:
            case DIV:
            case ASSIGN:
                alive = false;
                for (j = i + 1; j <= bbtab[b].end; j++) {
                    switch (stmtab[j].kind) {
                        case ADD:
                        case SUB:
                        case MUL:
                        case DIV:
                            if (stmtab[j].arg1 == stmtab[i].res || stmtab[j].arg2 == stmtab[i].res) {
                                alive = true;
                            }
                            break;
                        case ASSIGN:
                            if (stmtab[j].arg == stmtab[i].res) {
                                alive = true;
                            }
                            break;
                        case RETURN:
                            if (stmtab[j].ret == stmtab[i].res) {
                                alive = true;
                            }
                            break;
                        case WRITE:
                            if (stmtab[j].arg == stmtab[i].res) {
                                alive = true;
                            }
                            break;
                        case CALL:
                            for (k = 0; k < stmtab[j].argsNum; k++) {
                                if (stmtab[j].args[k] == stmtab[i].res) {
                                    alive = true;
                                    break;
                                }
                            }
                            break;
                        default: break;
                    }
                }
                if (!alive) {
                    stmtab[i].kind = NOP;
                }
                break;
            default: break;
        }
    }
}

void constFolding(bb_t b) {
    static int value[MAX_TABLE_LEN];
    static bool hasValue[MAX_TABLE_LEN];
    int k;
    for (k = 0; k < MAX_TABLE_LEN; k++) {
        hasValue[k] = false;
    }
    stmt_t i;
    int tmp;
    for (i = bbtab[b].begin; i <= bbtab[b].end; i++) {
        switch (stmtab[i].kind) {
            case ASSIGN:
                if (hasValue[stmtab[i].arg]) {
                    stmtab[i].arg = getOrCreateDirectRef(getOrCreateConstant(value[stmtab[i].arg]));
                }
                if (oprtab[reftab[stmtab[i].arg].opr].kind == CONSTANT && reftab[stmtab[i].res].kind == DIRECT) {
                    value[stmtab[i].res] = oprtab[reftab[stmtab[i].arg].opr].value;
                    hasValue[stmtab[i].res] = true;
                } else {
                    hasValue[stmtab[i].res] = false;
                }
                break;
            case ADD:
                if (hasValue[stmtab[i].arg1]) {
                    stmtab[i].arg1 = getOrCreateDirectRef(getOrCreateConstant(value[stmtab[i].arg1]));
                }
                if (hasValue[stmtab[i].arg2]) {
                    stmtab[i].arg2 = getOrCreateDirectRef(getOrCreateConstant(value[stmtab[i].arg2]));
                }
                if (oprtab[reftab[stmtab[i].arg1].opr].kind == CONSTANT 
                    && oprtab[reftab[stmtab[i].arg2].opr].kind == CONSTANT) {
                    tmp = oprtab[reftab[stmtab[i].arg1].opr].value 
                        + oprtab[reftab[stmtab[i].arg2].opr].value;
                    stmtab[i].kind = ASSIGN;
                    stmtab[i].arg = getOrCreateDirectRef(getOrCreateConstant(tmp));
                    if (reftab[stmtab[i].res].kind == DIRECT) {
                        value[stmtab[i].res] = tmp;
                        hasValue[stmtab[i].res] = true;
                    }
                } else {
                    hasValue[stmtab[i].res] = false;
                }
                break;
            case SUB:
                if (hasValue[stmtab[i].arg1]) {
                    stmtab[i].arg1 = getOrCreateDirectRef(getOrCreateConstant(value[stmtab[i].arg1]));
                }
                if (hasValue[stmtab[i].arg2]) {
                    stmtab[i].arg2 = getOrCreateDirectRef(getOrCreateConstant(value[stmtab[i].arg2]));
                }
                if (oprtab[reftab[stmtab[i].arg1].opr].kind == CONSTANT 
                    && oprtab[reftab[stmtab[i].arg2].opr].kind == CONSTANT) {
                    tmp = oprtab[reftab[stmtab[i].arg1].opr].value 
                        - oprtab[reftab[stmtab[i].arg2].opr].value;
                    stmtab[i].kind = ASSIGN;
                    stmtab[i].arg = getOrCreateDirectRef(getOrCreateConstant(tmp));
                    if (reftab[stmtab[i].res].kind == DIRECT) {
                        value[stmtab[i].res] = tmp;
                        hasValue[stmtab[i].res] = true;
                    }
                } else {
                    hasValue[stmtab[i].res] = false;
                }
                break;
            case MUL:
                if (hasValue[stmtab[i].arg1]) {
                    stmtab[i].arg1 = getOrCreateDirectRef(getOrCreateConstant(value[stmtab[i].arg1]));
                }
                if (hasValue[stmtab[i].arg2]) {
                    stmtab[i].arg2 = getOrCreateDirectRef(getOrCreateConstant(value[stmtab[i].arg2]));
                }
                if (oprtab[reftab[stmtab[i].arg1].opr].kind == CONSTANT 
                    && oprtab[reftab[stmtab[i].arg2].opr].kind == CONSTANT) {
                    tmp = oprtab[reftab[stmtab[i].arg1].opr].value 
                        * oprtab[reftab[stmtab[i].arg2].opr].value;
                    stmtab[i].kind = ASSIGN;
                    stmtab[i].arg = getOrCreateDirectRef(getOrCreateConstant(tmp));
                    if (reftab[stmtab[i].res].kind == DIRECT) {
                        value[stmtab[i].res] = tmp;
                        hasValue[stmtab[i].res] = true;
                    }
                } else {
                    hasValue[stmtab[i].res] = false;
                }
                break;
            case DIV:
                if (hasValue[stmtab[i].arg1]) {
                    stmtab[i].arg1 = getOrCreateDirectRef(getOrCreateConstant(value[stmtab[i].arg1]));
                }
                if (hasValue[stmtab[i].arg2]) {
                    stmtab[i].arg2 = getOrCreateDirectRef(getOrCreateConstant(value[stmtab[i].arg2]));
                }
                if (oprtab[reftab[stmtab[i].arg1].opr].kind == CONSTANT 
                    && oprtab[reftab[stmtab[i].arg2].opr].kind == CONSTANT) {
                    tmp = oprtab[reftab[stmtab[i].arg1].opr].value 
                        / oprtab[reftab[stmtab[i].arg2].opr].value;
                    stmtab[i].kind = ASSIGN;
                    stmtab[i].arg = getOrCreateDirectRef(getOrCreateConstant(tmp));
                    if (reftab[stmtab[i].res].kind == DIRECT) {
                        value[stmtab[i].res] = tmp;
                        hasValue[stmtab[i].res] = true;
                    }
                } else {
                    hasValue[stmtab[i].res] = false;
                }
                break;
            case WRITE:
                if (reftab[stmtab[i].arg].kind != DIRECT) {
                    break;
                }
                if (hasValue[stmtab[i].arg]) {
                    stmtab[i].arg = getOrCreateDirectRef(getOrCreateConstant(value[stmtab[i].arg]));
                }
                break;
            default: break;
        }
    }
}

