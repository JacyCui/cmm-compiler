#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "ir.h"

int errorno = 0;

char *symtab[MAX_TABLE_LEN];

int symtabSize = 0;

sym_t installID(const char* yytext, int yyleng) {
    sym_t i;
    for (i = 0; i < symtabSize; i++) {
        if (strcmp(symtab[i], yytext) == 0) {
            return i;
        }
    }
    symtab[symtabSize] = (char *)malloc(yyleng + 1);
    strcpy(symtab[symtabSize], yytext);
    return symtabSize++;
}

void freeSymtab() {
    sym_t i;
    for (i = 0; i < symtabSize; i++) {
        free(symtab[i]);
    }
}

oprtable_t oprtab[MAX_TABLE_LEN];

int oprtabSize = 0;

operand_t getOrCreateByName(sym_t name) {
    operand_t i;
    for (i = 0; i < oprtabSize; i++) {
        if (oprtab[i].kind == VAR && oprtab[i].varName == name) {
            return i;
        }
        if (oprtab[i].kind == ARRAY && oprtab[i].arrayName == name) {
            return i;
        }
    }
    oprtab[oprtabSize].kind = VAR;
    oprtab[oprtabSize].varName = name;
    return oprtabSize++;
}

operand_t createArray(sym_t name, int size) {
    oprtab[oprtabSize].kind = ARRAY;
    oprtab[oprtabSize].arrayName = name;
    oprtab[oprtabSize].arraySize = size;
    return oprtabSize++;
}

operand_t getOrCreateConstant(int value) {
    operand_t i;
    for (i = 0; i < oprtabSize; i++) {
        if (oprtab[i].kind == CONSTANT && oprtab[i].value == value) {
            return i;
        }
    }
    oprtab[oprtabSize].kind = CONSTANT;
    oprtab[oprtabSize].value = value;
    return oprtabSize++;
}

reftable_t reftab[MAX_TABLE_LEN];

int reftabSize = 0;

ref_t getOrCreateDirectRef(operand_t opr) {
    ref_t i;
    for (i = 0; i < reftabSize; i++) {
        if (reftab[i].kind == DIRECT && reftab[i].opr == opr) {
            return i;
        }
    }
    reftab[reftabSize].kind = DIRECT;
    reftab[reftabSize].opr = opr;
    return reftabSize++;
}

ref_t getOrCreateAddressRef(operand_t opr) {
    ref_t i;
    for (i = 0; i < reftabSize; i++) {
        if (reftab[i].kind == ADDRESS && reftab[i].opr == opr) {
            return i;
        }
    }
    reftab[reftabSize].kind = ADDRESS;
    reftab[reftabSize].opr = opr;
    return reftabSize++;
}

ref_t getOrCreateDereferenceRef(operand_t opr) {
    ref_t i;
    for (i = 0; i < reftabSize; i++) {
        if (reftab[i].kind == DEREFERENCE && reftab[i].opr == opr) {
            return i;
        }
    }
    reftab[reftabSize].kind = DEREFERENCE;
    reftab[reftabSize].opr = opr;
    return reftabSize++;
}

stmtable_t stmtab[MAX_TABLE_LEN];

int stmtabSize = 0;

stmt_t addDEC(operand_t array) {
    stmtab[stmtabSize].kind = DEC;
    stmtab[stmtabSize].array = array;
    return stmtabSize++;
}

stmt_t addCALL(sym_t funcName, int argsNum, ref_t argsBuffer[], ref_t ret) {
    stmtab[stmtabSize].kind = CALL;
    stmtab[stmtabSize].funcName = funcName;
    stmtab[stmtabSize].argsNum = argsNum;
    memcpy(stmtab[stmtabSize].args, argsBuffer, argsNum * sizeof(ref_t));
    stmtab[stmtabSize].ret = ret;
    return stmtabSize++;
}

stmt_t addLABEL(sym_t labelName) {
    stmtab[stmtabSize].kind = LABEL;
    stmtab[stmtabSize].labelName = labelName;
    return stmtabSize++;
}

stmt_t addASSIGN(ref_t res, ref_t arg) {
    stmtab[stmtabSize].kind = ASSIGN;
    stmtab[stmtabSize].res = res;
    stmtab[stmtabSize].arg = arg;
    return stmtabSize++;
}

stmt_t addADD(ref_t res, ref_t arg1, ref_t arg2) {
    stmtab[stmtabSize].kind = ADD;
    stmtab[stmtabSize].res = res;
    stmtab[stmtabSize].arg1 = arg1;
    stmtab[stmtabSize].arg2 = arg2;
    return stmtabSize++;
}

stmt_t addSUB(ref_t res, ref_t arg1, ref_t arg2) {
    stmtab[stmtabSize].kind = SUB;
    stmtab[stmtabSize].res = res;
    stmtab[stmtabSize].arg1 = arg1;
    stmtab[stmtabSize].arg2 = arg2;
    return stmtabSize++;
}

stmt_t addMUL(ref_t res, ref_t arg1, ref_t arg2) {
    stmtab[stmtabSize].kind = MUL;
    stmtab[stmtabSize].res = res;
    stmtab[stmtabSize].arg1 = arg1;
    stmtab[stmtabSize].arg2 = arg2;
    return stmtabSize++;
}

stmt_t addDIV(ref_t res, ref_t arg1, ref_t arg2) {
    stmtab[stmtabSize].kind = DIV;
    stmtab[stmtabSize].res = res;
    stmtab[stmtabSize].arg1 = arg1;
    stmtab[stmtabSize].arg2 = arg2;
    return stmtabSize++;
}

stmt_t addGOTO(sym_t target) {
    stmtab[stmtabSize].kind = GOTO;
    stmtab[stmtabSize].target = target;
    return stmtabSize++;
}

stmt_t addIFGOTO(ref_t arg1, relop_t relop, ref_t arg2, sym_t target) {
    stmtab[stmtabSize].kind = IFGOTO;
    stmtab[stmtabSize].arg1 = arg1;
    stmtab[stmtabSize].relop = relop;
    stmtab[stmtabSize].arg2 = arg2;
    stmtab[stmtabSize].target = target;
    return stmtabSize++;
}

stmt_t addRETURN(ref_t ret) {
    stmtab[stmtabSize].kind = RETURN;
    stmtab[stmtabSize].ret = ret;
    return stmtabSize++;
}

stmt_t addREAD(ref_t arg) {
    stmtab[stmtabSize].kind = READ;
    stmtab[stmtabSize].arg = arg;
    return stmtabSize++;
}

stmt_t addWRITE(ref_t arg) {
    stmtab[stmtabSize].kind = WRITE;
    stmtab[stmtabSize].arg = arg;
    return stmtabSize++;
}

functable_t functab[MAX_TABLE_LEN];

int functabSize = 0;

func_t createEmptyFunction(sym_t funcName) {
    functab[functabSize].funcName = funcName;
    functab[functabSize].paramNum = 0;
    return functabSize++;
}

static void dumpOpr(FILE* fd, operand_t o) {
    switch (oprtab[o].kind) {
        case CONSTANT: fprintf(fd, "#%d", oprtab[o].value); break;
        case VAR: fprintf(fd, "%s", symtab[oprtab[o].varName]); break;
        case ARRAY: fprintf(fd, "%s", symtab[oprtab[o].arrayName]); break;
    }
}

static void dumpRef(FILE* fd, ref_t r) {
    switch (reftab[r].kind) {
        case ADDRESS: fprintf(fd, "&"); break;
        case DEREFERENCE: fprintf(fd, "*"); break;
        default: break;
    }
    dumpOpr(fd, reftab[r].opr);
}

static void dumpStmt(FILE *fd, stmt_t k) {
    ref_t i;
    switch (stmtab[k].kind) {
        case DEC:
            assert(oprtab[stmtab[k].array].kind == ARRAY);
            fprintf(fd, "DEC %s %d\n", symtab[oprtab[stmtab[k].array].arrayName], 
                oprtab[stmtab[k].array].arraySize); 
            break;
        case CALL:
            for (i = 0; i < stmtab[k].argsNum; i++) {
                fprintf(fd, "ARG ");
                dumpRef(fd, stmtab[k].args[i]);
                fprintf(fd, "\n");
            }
            dumpRef(fd, stmtab[k].ret);
            fprintf(fd, " := CALL %s\n", symtab[stmtab[k].funcName]);
            break;
        case LABEL:
            fprintf(fd, "LABEL %s :\n", symtab[stmtab[k].labelName]);
            break;
        case ASSIGN:
            dumpRef(fd, stmtab[k].res);
            fprintf(fd, " := ");
            dumpRef(fd, stmtab[k].arg);
            fprintf(fd, "\n");
            break;
        case ADD:
            dumpRef(fd, stmtab[k].res);
            fprintf(fd, " := ");
            dumpRef(fd, stmtab[k].arg1);
            fprintf(fd, " + ");
            dumpRef(fd, stmtab[k].arg2);
            fprintf(fd, "\n");
            break;
        case SUB:
            dumpRef(fd, stmtab[k].res);
            fprintf(fd, " := ");
            dumpRef(fd, stmtab[k].arg1);
            fprintf(fd, " - ");
            dumpRef(fd, stmtab[k].arg2);
            fprintf(fd, "\n");
            break;
        case MUL:
            dumpRef(fd, stmtab[k].res);
            fprintf(fd, " := ");
            dumpRef(fd, stmtab[k].arg1);
            fprintf(fd, " * ");
            dumpRef(fd, stmtab[k].arg2);
            fprintf(fd, "\n");
            break;
        case DIV:
            dumpRef(fd, stmtab[k].res);
            fprintf(fd, " := ");
            dumpRef(fd, stmtab[k].arg1);
            fprintf(fd, " / ");
            dumpRef(fd, stmtab[k].arg2);
            fprintf(fd, "\n");
            break;
        case GOTO:
            fprintf(fd, "GOTO %s\n", symtab[stmtab[k].target]);
            break;
        case IFGOTO:
            fprintf(fd, "IF ");
            dumpRef(fd, stmtab[k].arg1);
            switch (stmtab[k].relop) {
                case GE: fprintf(fd, " >= "); break;
                case LE: fprintf(fd, " <= "); break;
                case EQ: fprintf(fd, " == "); break;
                case NE: fprintf(fd, " != "); break;
                case LT: fprintf(fd, " < "); break;
                case GT: fprintf(fd, " > "); break;
            }
            dumpRef(fd, stmtab[k].arg2);
            fprintf(fd, " GOTO %s\n", symtab[stmtab[k].target]);
            break;
        case RETURN:
            fprintf(fd, "RETURN ");
            dumpRef(fd, stmtab[k].ret);
            fprintf(fd, "\n");
            break;
        case READ:
            fprintf(fd, "READ ");
            dumpRef(fd, stmtab[k].arg);
            fprintf(fd, "\n");
            break;
        case WRITE:
            fprintf(fd, "WRITE ");
            dumpRef(fd, stmtab[k].arg);
            fprintf(fd, "\n");
            break;
    }
}

static void dumpFunctionIR(FILE *fd, func_t i) {
    operand_t j;
    stmt_t k;
    fprintf(fd, "FUNCTION %s :\n", symtab[functab[i].funcName]);
    for (j = 0; j < functab[i].paramNum; j++) {
        assert(oprtab[functab[i].params[j]].kind == VAR);
        fprintf(fd, "PARAM ");
        dumpOpr(fd, functab[i].params[j]);
        fprintf(fd, "\n");
    }
    for (k = functab[i].irBegin; k <= functab[i].irEnd; k++) {
        dumpStmt(fd, k);
    }
}

void dumpIR(FILE *fd) {
    func_t i;
    for (i = 0; i < functabSize; i++) {
        dumpFunctionIR(fd, i);
    }
}
