#include <assert.h>
#include <stdio.h>

#include "config.h"
#include "ir.h"

label_t max_label = 0;

label_t newLabel() {
    return max_label++;
}

int tempNo = 0;
operandtable_t operandtab[MAX_TABLE_LEN * 2];

operand_t operandtab_size = 0;

operand_t getOrCreateVarByName(name_t name) {
    operand_t i;
    for (i = 0; i < operandtab_size; i++) {
        if (operandtab[i].kind == DECL_VAR && operandtab[i].name == name) {
            return i;
        }
    }
    operandtab[operandtab_size].kind = DECL_VAR;
    operandtab[operandtab_size].name = name;
    return operandtab_size++;
}

operand_t newTemp() {
    operandtab[operandtab_size].kind = TEMP_VAR;
    operandtab[operandtab_size].tempNo = tempNo++;
    return operandtab_size++;
}

operand_t getOrCreateConstant(int value) {
    operand_t i;
    for (i = 0; i < operandtab_size; i++) {
        if (operandtab[i].kind == CONSTANT && operandtab[i].value == value) {
            return i;
        }
    }
    operandtab[operandtab_size].kind = CONSTANT;
    operandtab[operandtab_size].value = value;
    return operandtab_size++;
}

void printOperand(FILE* fd, operand_t i) {
    assert(fd);
    operandtable_t *operand = operandtab + i;
    switch (operand->kind) {
        case DECL_VAR: fprintf(fd, "%s", symtab[nametab[operand->name].sym].symbol);
        case CONSTANT: fprintf(fd, "#%d", operand->value);
        case TEMP_VAR: fprintf(fd, "t%d", operand->tempNo);
    }
}

irtable_t irtab[MAX_TABLE_LEN * 2];

ir_t irtab_size = 0;

ir_t IR_FUNCTION(sym_t funcName) {
    irtab[irtab_size].kind = FUNCTION;
    irtab[irtab_size].funcName = funcName;
    return irtab_size++;
}

ir_t IR_PARAM(operand_t param) {
    irtab[irtab_size].kind = PARAM;
    irtab[irtab_size].param = param;
    return irtab_size++;
}

ir_t IR_LABEL(label_t label) {
    irtab[irtab_size].kind = LABEL;
    irtab[irtab_size].label = label;
    return irtab_size++;
}

ir_t IR_DEC(operand_t array, int size) {
    irtab[irtab_size].kind = DEC;
    irtab[irtab_size].array = array;
    irtab[irtab_size].size = size;
    return irtab_size++;
}

ir_t IR_ASSIGN(operand_t dest, operand_t src) {
    irtab[irtab_size].kind = ASSIGN;
    irtab[irtab_size].dest = dest;
    irtab[irtab_size].src = src;
    return irtab_size++;
}

ir_t IR_GET_ADDR(operand_t dest, operand_t src) {
    irtab[irtab_size].kind = GET_ADDR;
    irtab[irtab_size].dest = dest;
    irtab[irtab_size].src = src;
    return irtab_size++;
}

ir_t IR_LOAD(operand_t dest, operand_t src) {
    irtab[irtab_size].kind = LOAD;
    irtab[irtab_size].dest = dest;
    irtab[irtab_size].src = src;
    return irtab_size++;
}

ir_t IR_STORE(operand_t dest, operand_t src) {
    irtab[irtab_size].kind = STORE;
    irtab[irtab_size].dest = dest;
    irtab[irtab_size].src = src;
    return irtab_size++;
}

ir_t IR_ADD(operand_t res, operand_t opr1, operand_t opr2) {
    irtab[irtab_size].kind = ADD;
    irtab[irtab_size].res = res;
    irtab[irtab_size].opr1 = opr1;
    irtab[irtab_size].opr2 = opr2;
    return irtab_size++;
}

ir_t IR_SUB(operand_t res, operand_t opr1, operand_t opr2) {
    irtab[irtab_size].kind = SUB;
    irtab[irtab_size].res = res;
    irtab[irtab_size].opr1 = opr1;
    irtab[irtab_size].opr2 = opr2;
    return irtab_size++;
}

ir_t IR_MUL(operand_t res, operand_t opr1, operand_t opr2) {
    irtab[irtab_size].kind = MUL;
    irtab[irtab_size].res = res;
    irtab[irtab_size].opr1 = opr1;
    irtab[irtab_size].opr2 = opr2;
    return irtab_size++;
}

ir_t IR_DIV(operand_t res, operand_t opr1, operand_t opr2) {
    irtab[irtab_size].kind = DIV;
    irtab[irtab_size].res = res;
    irtab[irtab_size].opr1 = opr1;
    irtab[irtab_size].opr2 = opr2;
    return irtab_size++;
}

static void printOneIR(FILE* fd, ir_t i);

void printIR(FILE* fd) {
    assert(fd);
    ir_t i;
    for (i = 0; i < irtab_size; i++) {
        printOneIR(fd, i);
    }
}

static void printOneIR(FILE* fd, ir_t i) {
    irtable_t *ir = irtab + i;
    switch (ir->kind) {
        case LABEL: fprintf(fd, "LABEL label%d :\n", ir->label); break;
        case FUNCTION: fprintf(fd, "FUNCTION %s :\n", symtab[ir->funcName].symbol); break;
        case ASSIGN:
            printOperand(fd, ir->dest);
            fprintf(fd, " := ");
            printOperand(fd, ir->src);
            fprintf(fd, "\n");
            break;
        case ADD:
            printOperand(fd, ir->res);
            fprintf(fd, " := ");
            printOperand(fd, ir->opr1);
            fprintf(fd, " + ");
            printOperand(fd, ir->opr2);
            fprintf(fd, "\n");
            break;
        case SUB:
            printOperand(fd, ir->res);
            fprintf(fd, " := ");
            printOperand(fd, ir->opr1);
            fprintf(fd, " - ");
            printOperand(fd, ir->opr2);
            fprintf(fd, "\n");
            break;
        case MUL:
            printOperand(fd, ir->res);
            fprintf(fd, " := ");
            printOperand(fd, ir->opr1);
            fprintf(fd, " * ");
            printOperand(fd, ir->opr2);
            fprintf(fd, "\n");
            break;
        case DIV:
            printOperand(fd, ir->res);
            fprintf(fd, " := ");
            printOperand(fd, ir->opr1);
            fprintf(fd, " / ");
            printOperand(fd, ir->opr2);
            fprintf(fd, "\n");
            break;
        case GET_ADDR:
            printOperand(fd, ir->dest);
            fprintf(fd, " := &");
            printOperand(fd, ir->src);
            fprintf(fd, "\n");
            break;
        case LOAD:
            printOperand(fd, ir->dest);
            fprintf(fd, " := *");
            printOperand(fd, ir->src);
            fprintf(fd, "\n");
            break;
        case STORE:
            fprintf(fd, "*");
            printOperand(fd, ir->dest);
            fprintf(fd, " := ");
            printOperand(fd, ir->src);
            fprintf(fd, "\n");
            break;
        case JUMP: fprintf(fd, "GOTO label%d\n", ir->label); break;
        case BRANCH_EQ:
            fprintf(fd, "IF ");
            printOperand(fd, ir->opr1);
            fprintf(fd, " == ");
            printOperand(fd, ir->opr2);
            fprintf(fd, " GOTO label%d\n", ir->target);
            break;
        case BRANCH_NE:
            fprintf(fd, "IF ");
            printOperand(fd, ir->opr1);
            fprintf(fd, " != ");
            printOperand(fd, ir->opr2);
            fprintf(fd, " GOTO label%d\n", ir->target);
            break;
        case BRANCH_LT:
            fprintf(fd, "IF ");
            printOperand(fd, ir->opr1);
            fprintf(fd, " < ");
            printOperand(fd, ir->opr2);
            fprintf(fd, " GOTO label%d\n", ir->target);
            break;
        case BRANCH_LE:
            fprintf(fd, "IF ");
            printOperand(fd, ir->opr1);
            fprintf(fd, " <= ");
            printOperand(fd, ir->opr2);
            fprintf(fd, " GOTO label%d\n", ir->target);
            break;
        case BRANCH_GT:
            fprintf(fd, "IF ");
            printOperand(fd, ir->opr1);
            fprintf(fd, " > ");
            printOperand(fd, ir->opr2);
            fprintf(fd, " GOTO label%d\n", ir->target);
            break;
        case BRANCH_GE:
            fprintf(fd, "IF ");
            printOperand(fd, ir->opr1);
            fprintf(fd, " >= ");
            printOperand(fd, ir->opr2);
            fprintf(fd, " GOTO label%d\n", ir->target);
            break;
        case RETURN:
            fprintf(fd, "RETURN ");
            printOperand(fd, ir->ret);
            fprintf(fd, "\n");
            break;
        case DEC:
            fprintf(fd, "DEC ");
            printOperand(fd, ir->array);
            fprintf(fd, " %d\n", ir->size);
            break;
        case ARG:
            fprintf(fd, "ARG ");
            printOperand(fd, ir->arg);
            fprintf(fd, "\n");
            break;
        case CALL:
            printOperand(fd, ir->ret);
            fprintf(fd, " := CALL %s\n", symtab[ir->funcName].symbol);
            break;
        case PARAM:
            fprintf(fd, "PARAM ");
            printOperand(fd, ir->param);
            fprintf(fd, "\n");
            break;
        case READ:
            fprintf(fd, "READ ");
            printOperand(fd, ir->arg);
            fprintf(fd, "\n");
            break;
        case WRITE:
            fprintf(fd, "WRITE ");
            printOperand(fd, ir->arg);
            fprintf(fd, "\n");
            break;
    }
}
