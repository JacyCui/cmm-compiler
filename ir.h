#ifndef IR_H
#define IR_H

#include "semantic.h"
#include <stdio.h>

typedef int label_t;
label_t newLabel();

typedef int operand_t;

typedef int ir_t;

typedef struct operandtable_t {
    enum { DECL_VAR, CONSTANT, TEMP_VAR } kind;
    union {
        name_t name; // for DECL_VAR
        int value; // for CONSTANT
        int tempNo; // for TEMP_VAR
    };
} operandtable_t;

extern operandtable_t operandtab[];

operand_t getOrCreateVarByName(name_t name);
operand_t newTemp();
operand_t getOrCreateConstant(int value);

typedef struct irtable_t {
    enum {
        LABEL,
        FUNCTION,
        ASSIGN,
        ADD,
        SUB,
        MUL,
        DDIV,
        GET_ADDR,
        LOAD,
        STORE,
        JUMP,
        BRANCH_EQ,
        BRANCH_NE,
        BRANCH_LT,
        BRANCH_LE,
        BRANCH_GT,
        BRANCH_GE,
        RRETURN,
        DEC,
        ARG,
        CALL,
        PARAM,
        READ,
        WRITE
    } kind;
    union {
        label_t label; // for LABEL, JUMP
        struct {
            sym_t funcName; // for FUNCTION
            operand_t ret; // for RETURN
        }; // for CALL
        struct {
            operand_t dest;
            operand_t src;
        }; // for ASSIGN, GET_ADDR, LOAD, STORE
        struct {
            union {
                operand_t res; // for ADD, SUB, MUL, DIV
                label_t target; 
                // for BRANCH_EQ, BRANCH_NE, BRANCH_LT, BRANCH_LE, BRANCH_GT, BRANCH_GE
            };
            operand_t opr1;
            operand_t opr2;
        };
        operand_t param; // for PARAM
        operand_t arg; // for ARG, READ, WRITE
        struct {
            operand_t array;
            int size;
        }; // for DEC
    };
} irtable_t;

extern irtable_t irtab[];

ir_t IR_FUNCTION(sym_t funcName);
ir_t IR_PARAM(operand_t param);
ir_t IR_LABEL(label_t label);
ir_t IR_JUMP(label_t label);
ir_t IR_DEC(operand_t array, int size);
ir_t IR_ASSIGN(operand_t dest, operand_t src);
ir_t IR_GET_ADDR(operand_t dest, operand_t src);
ir_t IR_LOAD(operand_t dest, operand_t src);
ir_t IR_STORE(operand_t dest, operand_t src);
ir_t IR_ADD(operand_t res, operand_t opr1, operand_t opr2);
ir_t IR_SUB(operand_t res, operand_t opr1, operand_t opr2);
ir_t IR_MUL(operand_t res, operand_t opr1, operand_t opr2);
ir_t IR_DIV(operand_t res, operand_t opr1, operand_t opr2);
ir_t IR_BRANCH_EQ(label_t target, operand_t opr1, operand_t opr2);
ir_t IR_BRANCH_NE(label_t target, operand_t opr1, operand_t opr2);
ir_t IR_BRANCH_LT(label_t target, operand_t opr1, operand_t opr2);
ir_t IR_BRANCH_LE(label_t target, operand_t opr1, operand_t opr2);
ir_t IR_BRANCH_GT(label_t target, operand_t opr1, operand_t opr2);
ir_t IR_BRANCH_GE(label_t target, operand_t opr1, operand_t opr2);
ir_t IR_ARG(operand_t arg);
ir_t IR_READ(operand_t arg);
ir_t IR_WRITE(operand_t arg);
ir_t IR_CALL(operand_t ret, sym_t funcName);
ir_t IR_RETURN(operand_t ret);

void dumpIR(FILE *fd);

#endif
