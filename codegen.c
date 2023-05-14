#include "codegen.h"

#include "ast.h"
#include "config.h"
#include "ir.h"
#include "semantic.h"
#include "symbol.h"
#include "syntax.tab.h"
#include <stdio.h>

int frameSize[MAX_TABLE_LEN] = {0};

int offsetToFP[MAX_TABLE_LEN];

static void calculateFrameSizeAndOffsetToFP();

// void translateIRToCode(FILE *fd) {
//     calculateFrameSizeAndOffsetToFP();
//     operand_t i;
//     operandtable_t *op;
//     for (i = 0; i < operandtab_size; i++) {
//         op = operandtab + i;
//         if (op->kind == DECL_VAR) {
//             fprintf(fd, "v%d: %d\n", op->varNo, offsetToFP[i]);
//         } else if (op->kind == TEMP_VAR) {
//             fprintf(fd, "t%d: %d\n", op->tempNo, offsetToFP[i]);
//         } else {
//             fprintf(fd, "const: %d\n", op->value);
//         }
//     }
//     func_t j;
//     for (j = 0; j < func_table_size; j++) {
//         fprintf(fd, "%s framesize: %d\n", symtab[functab[j].sym].symbol, frameSize[functab[j].sym]);
//     }
// }

void translateIRToCode(FILE *fd) {
    calculateFrameSizeAndOffsetToFP();
    fprintf(fd, "%s", asmHeader);
    ir_t i;
    irtable_t *ir;
    sym_t currentFunctionName;
    int currentFrameSize;
    for (i = 0; i < irtab_size; i++) {
        ir = irtab + i;
        switch (ir->kind) {
            case FUNCTION:
                currentFunctionName = ir->funcName;
                currentFrameSize = frameSize[currentFunctionName];
                fprintf(fd, "\n%s:\n\
    subu $sp, $sp, %d # framesize = %d\n\
    sw $ra, %d($sp) # store $ra\n\
    sw $fp, %d($sp) # store $fp\n\
    addi $fp, $sp, %d # update $fp\n\
    # ", symtab[currentFunctionName].symbol, currentFrameSize, currentFrameSize, 
                    currentFrameSize - 4, currentFrameSize - 8, currentFrameSize);
                break;
            case RRETURN:
                if (operandtab[ir->ret].kind == CONSTANT) {
                    fprintf(fd, "    li $v0, %d\n", operandtab[ir->ret].value);
                } else {
                    fprintf(fd, "    lw $v0, %d($fp)\n", offsetToFP[ir->ret]);
                }
                fprintf(fd, "    lw $ra, -4($fp)\n\
    lw $fp, -8($fp)\n\
    addi $sp, $sp, %d\n\
    jr $ra # ", currentFrameSize);
                break;
            case LABEL:
                fprintf(fd, "label%d: # ", ir->label);
                break;
            case ASSIGN:
                if (operandtab[ir->src].kind == CONSTANT) {
                    fprintf(fd, "    li $t0, %d\n", operandtab[ir->src].value);
                } else {
                    fprintf(fd, "    lw $t0, %d($fp)\n", offsetToFP[ir->src]);
                }
                fprintf(fd, "    sw $t0, %d($fp) # ", offsetToFP[ir->dest]);
                break;
            case ADD:
                if (operandtab[ir->opr1].kind == CONSTANT) {
                    fprintf(fd, "    li $t0, %d\n", operandtab[ir->opr1].value);
                } else {
                    fprintf(fd, "    lw $t0, %d($fp)\n", offsetToFP[ir->opr1]);
                }
                if (operandtab[ir->opr2].kind == CONSTANT) {
                    fprintf(fd, "    li $t1, %d\n", operandtab[ir->opr2].value);
                } else {
                    fprintf(fd, "    lw $t1, %d($fp)\n", offsetToFP[ir->opr2]);
                }
                fprintf(fd, "    add $t2, $t0, $t1\n");
                fprintf(fd, "    sw $t2, %d($fp) # ", offsetToFP[ir->res]);
                break;
            case SUB:
                if (operandtab[ir->opr1].kind == CONSTANT) {
                    fprintf(fd, "    li $t0, %d\n", operandtab[ir->opr1].value);
                } else {
                    fprintf(fd, "    lw $t0, %d($fp)\n", offsetToFP[ir->opr1]);
                }
                if (operandtab[ir->opr2].kind == CONSTANT) {
                    fprintf(fd, "    li $t1, %d\n", operandtab[ir->opr2].value);
                } else {
                    fprintf(fd, "    lw $t1, %d($fp)\n", offsetToFP[ir->opr2]);
                }
                fprintf(fd, "    sub $t2, $t0, $t1\n");
                fprintf(fd, "    sw $t2, %d($fp) # ", offsetToFP[ir->res]);
                break;
            case MUL:
                if (operandtab[ir->opr1].kind == CONSTANT) {
                    fprintf(fd, "    li $t0, %d\n", operandtab[ir->opr1].value);
                } else {
                    fprintf(fd, "    lw $t0, %d($fp)\n", offsetToFP[ir->opr1]);
                }
                if (operandtab[ir->opr2].kind == CONSTANT) {
                    fprintf(fd, "    li $t1, %d\n", operandtab[ir->opr2].value);
                } else {
                    fprintf(fd, "    lw $t1, %d($fp)\n", offsetToFP[ir->opr2]);
                }
                fprintf(fd, "    mul $t2, $t0, $t1\n");
                fprintf(fd, "    sw $t2, %d($fp) # ", offsetToFP[ir->res]);
                break;
            case DDIV:
                if (operandtab[ir->opr1].kind == CONSTANT) {
                    fprintf(fd, "    li $t0, %d\n", operandtab[ir->opr1].value);
                } else {
                    fprintf(fd, "    lw $t0, %d($fp)\n", offsetToFP[ir->opr1]);
                }
                if (operandtab[ir->opr2].kind == CONSTANT) {
                    fprintf(fd, "    li $t1, %d\n", operandtab[ir->opr2].value);
                } else {
                    fprintf(fd, "    lw $t1, %d($fp)\n", offsetToFP[ir->opr2]);
                }
                fprintf(fd, "    div $t0, $t1\n");
                fprintf(fd, "    mflo $t2\n");
                fprintf(fd, "    sw $t2, %d($fp) # ", offsetToFP[ir->res]);
                break;
            case GET_ADDR:
                fprintf(fd, "    la $t0, %d($fp)\n\
    sw $t0, %d($fp) # ", offsetToFP[ir->src], offsetToFP[ir->dest]);
                break;
            case LOAD:
                fprintf(fd, "    lw $t0, %d($fp)\n\
    lw $t0, 0($t0)\n\
    sw $t0, %d($fp) # ", offsetToFP[ir->src], offsetToFP[ir->dest]);
                break;
            case STORE:
                fprintf(fd, "    lw $t0, %d($fp)\n\
    lw $t1, %d($fp)\n\
    sw $t0, 0($t1) # ", offsetToFP[ir->src], offsetToFP[ir->dest]);
                break;
            case JUMP: fprintf(fd, "    j label%d # ", ir->label); break;
            case BRANCH_EQ:
                if (operandtab[ir->opr1].kind == CONSTANT) {
                    fprintf(fd, "    li $t0, %d\n", operandtab[ir->opr1].value);
                } else {
                    fprintf(fd, "    lw $t0, %d($fp)\n", offsetToFP[ir->opr1]);
                }
                if (operandtab[ir->opr2].kind == CONSTANT) {
                    fprintf(fd, "    li $t1, %d\n", operandtab[ir->opr2].value);
                } else {
                    fprintf(fd, "    lw $t1, %d($fp)\n", offsetToFP[ir->opr2]);
                }
                fprintf(fd, "    beq $t0, $t1, label%d # ", ir->target);
                break;
            case BRANCH_NE:
                if (operandtab[ir->opr1].kind == CONSTANT) {
                    fprintf(fd, "    li $t0, %d\n", operandtab[ir->opr1].value);
                } else {
                    fprintf(fd, "    lw $t0, %d($fp)\n", offsetToFP[ir->opr1]);
                }
                if (operandtab[ir->opr2].kind == CONSTANT) {
                    fprintf(fd, "    li $t1, %d\n", operandtab[ir->opr2].value);
                } else {
                    fprintf(fd, "    lw $t1, %d($fp)\n", offsetToFP[ir->opr2]);
                }
                fprintf(fd, "    bne $t0, $t1, label%d # ", ir->target);
                break;
            case BRANCH_LT:
                if (operandtab[ir->opr1].kind == CONSTANT) {
                    fprintf(fd, "    li $t0, %d\n", operandtab[ir->opr1].value);
                } else {
                    fprintf(fd, "    lw $t0, %d($fp)\n", offsetToFP[ir->opr1]);
                }
                if (operandtab[ir->opr2].kind == CONSTANT) {
                    fprintf(fd, "    li $t1, %d\n", operandtab[ir->opr2].value);
                } else {
                    fprintf(fd, "    lw $t1, %d($fp)\n", offsetToFP[ir->opr2]);
                }
                fprintf(fd, "    blt $t0, $t1, label%d # ", ir->target);
                break;
            case BRANCH_LE:
                if (operandtab[ir->opr1].kind == CONSTANT) {
                    fprintf(fd, "    li $t0, %d\n", operandtab[ir->opr1].value);
                } else {
                    fprintf(fd, "    lw $t0, %d($fp)\n", offsetToFP[ir->opr1]);
                }
                if (operandtab[ir->opr2].kind == CONSTANT) {
                    fprintf(fd, "    li $t1, %d\n", operandtab[ir->opr2].value);
                } else {
                    fprintf(fd, "    lw $t1, %d($fp)\n", offsetToFP[ir->opr2]);
                }
                fprintf(fd, "    ble $t0, $t1, label%d # ", ir->target);
                break;
            case BRANCH_GT:
                if (operandtab[ir->opr1].kind == CONSTANT) {
                    fprintf(fd, "    li $t0, %d\n", operandtab[ir->opr1].value);
                } else {
                    fprintf(fd, "    lw $t0, %d($fp)\n", offsetToFP[ir->opr1]);
                }
                if (operandtab[ir->opr2].kind == CONSTANT) {
                    fprintf(fd, "    li $t1, %d\n", operandtab[ir->opr2].value);
                } else {
                    fprintf(fd, "    lw $t1, %d($fp)\n", offsetToFP[ir->opr2]);
                }
                fprintf(fd, "    bgt $t0, $t1, label%d # ", ir->target);
                break;
            case BRANCH_GE:
                if (operandtab[ir->opr1].kind == CONSTANT) {
                    fprintf(fd, "    li $t0, %d\n", operandtab[ir->opr1].value);
                } else {
                    fprintf(fd, "    lw $t0, %d($fp)\n", offsetToFP[ir->opr1]);
                }
                if (operandtab[ir->opr2].kind == CONSTANT) {
                    fprintf(fd, "    li $t1, %d\n", operandtab[ir->opr2].value);
                } else {
                    fprintf(fd, "    lw $t1, %d($fp)\n", offsetToFP[ir->opr2]);
                }
                fprintf(fd, "    bge $t0, $t1, label%d # ", ir->target);
                break;
            case ARG:
                fprintf(fd, "    subu $sp, $sp, 4\n");
                if (operandtab[ir->arg].kind == CONSTANT) {
                    fprintf(fd, "    li $t0, %d\n", operandtab[ir->arg].value);
                } else {
                    fprintf(fd, "    lw $t0, %d($fp)\n", offsetToFP[ir->arg]);
                }
                fprintf(fd, "    sw $t0, 0($sp) # ");
                break;
            case CALL:
                fprintf(fd, "    jal %s\n\
    addi $sp, %d\n\
    sw $v0, %d($fp) # ", symtab[ir->funcName].symbol, 
                    functab[getFunctionByName(ir->funcName)].param_num * 4, 
                    offsetToFP[ir->ret]);
                break;
            case READ:
                fprintf(fd, "    jal read\n\
    sw $v0, %d($fp) # ", offsetToFP[ir->arg]);
                break;
            case WRITE:
                if (operandtab[ir->arg].kind == CONSTANT) {
                    fprintf(fd, "    li $a0, %d\n", operandtab[ir->arg].value);
                } else {
                    fprintf(fd, "    lw $a0, %d($fp)\n", offsetToFP[ir->arg]);
                }
                fprintf(fd, "    jal write # ");
                break;
            default: fprintf(fd, "    # "); break;
        }
        dumpOneIR(fd, i);
    }
}


static void allocateLocalOprSpace(operand_t opr, int *currentFrameSize, int *currentNegativeOffsetToFP) {
    if(offsetToFP[opr] == -1 && operandtab[opr].kind != CONSTANT) {
        offsetToFP[opr] = *currentNegativeOffsetToFP;
        *currentNegativeOffsetToFP -= 4;
        *currentFrameSize += 4;
    }
}

static void calculateFrameSizeAndOffsetToFP() {
    int j;
    for (j = 0; j < MAX_TABLE_LEN; j++) {
        offsetToFP[j] = -1;
    }
    ir_t i;
    irtable_t *ir;
    sym_t currentFunctionName = -1;
    int currentFrameSize = 8; // for $ra and old $fp
    int currentPositiveOffsetToFP = 0;
    int currentNegativeOffsetToFP = -12;
    for (i = 0; i < irtab_size; i++) {
        ir = irtab + i;
        switch (ir->kind) {
            case FUNCTION: 
                if (currentFunctionName != -1) {
                    frameSize[currentFunctionName] = currentFrameSize;
                }
                currentFunctionName = ir->funcName;
                currentFrameSize = 8;
                currentPositiveOffsetToFP = 0;
                currentNegativeOffsetToFP = -12;
                break;
            case PARAM:
                offsetToFP[ir->param] = currentPositiveOffsetToFP;
                currentPositiveOffsetToFP += 4;
                break;
            case DEC:
                allocateLocalOprSpace(ir->array, &currentFrameSize, &currentNegativeOffsetToFP);
                currentNegativeOffsetToFP -= ir->size - 4;
                currentFrameSize += ir->size - 4;
                break;
            case ASSIGN:
            case GET_ADDR:
            case LOAD:
            case STORE:
                allocateLocalOprSpace(ir->src, &currentFrameSize, &currentNegativeOffsetToFP);
                allocateLocalOprSpace(ir->dest, &currentFrameSize, &currentNegativeOffsetToFP);
                break;
            case ADD:
            case SUB:
            case MUL:
            case DDIV:
                allocateLocalOprSpace(ir->opr1, &currentFrameSize, &currentNegativeOffsetToFP);
                allocateLocalOprSpace(ir->opr2, &currentFrameSize, &currentNegativeOffsetToFP);
                allocateLocalOprSpace(ir->res, &currentFrameSize, &currentNegativeOffsetToFP);
                break;
            case BRANCH_EQ:
            case BRANCH_NE:
            case BRANCH_LT:
            case BRANCH_LE:
            case BRANCH_GT:
            case BRANCH_GE:
                allocateLocalOprSpace(ir->opr1, &currentFrameSize, &currentNegativeOffsetToFP);
                allocateLocalOprSpace(ir->opr2, &currentFrameSize, &currentNegativeOffsetToFP);
                break;
            case RRETURN:
            case CALL:
                allocateLocalOprSpace(ir->ret, &currentFrameSize, &currentNegativeOffsetToFP);
                break;
            case ARG:
            case READ:
            case WRITE:
                allocateLocalOprSpace(ir->arg, &currentFrameSize, &currentNegativeOffsetToFP);
                break;
            default: break;
        }
    }
    frameSize[currentFunctionName] = currentFrameSize;
}

















const char* asmHeader = ".data\n\
_prompt: .asciiz \"Enter an integer:\"\n\
_ret: .asciiz \"\\n\"\n\
.globl main\n\
.text\n\
read:\n\
    li $v0, 4\n\
    la $a0, _prompt\n\
    syscall\n\
    li $v0, 5\n\
    syscall\n\
    jr $ra\n\
\n\
write:\n\
    li $v0, 1\n\
    syscall\n\
    li $v0, 4\n\
    la $a0, _ret\n\
    syscall\n\
    move $v0, $0\n\
    jr $ra\n";


