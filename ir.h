#ifndef IR_H
#define IR_H

#include <stdio.h>

#include "config.h"

extern int errorno;

/// ------- Symbol Table ---------

/**
 * @brief symbol table index
 */
typedef int sym_t;

extern char* symtab[]; ///< symbol table

extern int symtabSize; ///< the number of symbols

/**
 * @brief Install a symbol into the symbol table
 * @param yytext the symbol string
 * @param yyleng the string length
 * @return the id of the symbol
 */
sym_t installID(const char* yytext, int yyleng);

/**
 * @brief free the memory allocated to symbol table
 */
void freeSymtab();


/// ------- Operand Table ---------

/**
 * @brief operand table index
 */
typedef int operand_t;

/**
 * @brief operand table entry
 */
typedef struct {
    enum {
        CONSTANT, ///< integet constant
        VAR,  ///< integer variable
        ARRAY ///< integer array
    } kind; ///< operand kind
    union {
        int value; ///< constant value
        sym_t varName; ///< variable name
        struct {
            sym_t arrayName; ///< array name
            int arraySize; ///< array length
        };
    };
} oprtable_t;

extern oprtable_t oprtab[]; ///< operand table

extern int oprtabSize; ///< the number of symbols

/**
 * @brief get or created a variable or get an array by its name
 * @param name the name to be searched
 * @return the index of the operand
 */
operand_t getOrCreateByName(sym_t name);

/**
 * @brief create an array by its name and size
 * @param name the name of the array
 * @param size the name of the size
 * @return the index of the operand
 */
operand_t createArray(sym_t name, int size);

/**
 * @brief get or create a const by its value
 * @param value the value of the constant
 * @return the index of the operand
 */
operand_t getOrCreateConstant(int value);


/// ------- Reference Table ---------

typedef int ref_t;

/**
 * @brief reference of an operand
 */
typedef struct {
    enum {
        DIRECT, ///< direct access
        ADDRESS, ///< get address
        DEREFERENCE ///< dereference
    } kind; ///< way of accessing an operand
    operand_t opr;
} reftable_t;

extern reftable_t reftab[]; ///< reference table

extern int reftabSize; ///< size of reference table

/**
 * @brief get or create direct reference of an operand
 * @param opr operand index
 * @return reference index
 */
ref_t getOrCreateDirectRef(operand_t opr);

/**
 * @brief get or create addressing reference of an operand
 * @param opr operand index
 * @return reference index
 */
ref_t getOrCreateAddressRef(operand_t opr);

/**
 * @brief get or create dereference reference of an operand
 * @param opr operand index
 * @return reference index
 */
ref_t getOrCreateDereferenceRef(operand_t opr);


/// ------- Statement Table ---------

/**
 * @brief statement table index
 */
typedef int stmt_t;

/**
 * @brief function table index
 */
typedef int func_t;

/**
 * @brief type of relational operations
 */
typedef enum {GE, LE, EQ, NE, LT, GT} relop_t;

/**
 * @brief statement table entry
 */
typedef struct {
    enum {
        DEC, ///< DEC x [size]
        CALL, ///< x := CALL f
        LABEL, ///< LABEL x :
        ASSIGN, ///< x := y
        ADD, ///< x := y + z
        SUB, ///< x := y - z
        MUL, ///< x := y * z
        DIV, ///< x := y / z
        GOTO, ///< GOTO x
        IFGOTO, ///< IF x [relop] y GOTO z
        RETURN, ///< RETURN x
        READ, ///< READ x
        WRITE ///< READ y
    } kind;
    union {
        operand_t array; ///< for DEC
        struct {
            sym_t funcName;
            int argsNum;
            ref_t args[MAX_PARAM_NUM];
            ref_t ret;
        }; ///< for ARG, CALL, RETURN
        struct {
            sym_t target;
            union {
                ref_t res;
                relop_t relop;
            };
            union {
                ref_t arg;
                ref_t arg1;
            };
            ref_t arg2;
        }; ///< for ASSIGN, ADD, SUB, MUL, DIV, GOTO, IFGOTO, READ, WRITE
        sym_t labelName; ///< for LABEL
    };
} stmtable_t;

extern stmtable_t stmtab[]; ///< statement table

extern int stmtabSize; ///< number of statements

// statement constructors

stmt_t addDEC(operand_t array);
stmt_t addCALL(sym_t funcName, int argsNum, ref_t argsBuffer[], ref_t ret);
stmt_t addLABEL(sym_t labelName);
stmt_t addASSIGN(ref_t res, ref_t arg);
stmt_t addADD(ref_t res, ref_t arg1, ref_t arg2);
stmt_t addSUB(ref_t res, ref_t arg1, ref_t arg2);
stmt_t addMUL(ref_t res, ref_t arg1, ref_t arg2);
stmt_t addDIV(ref_t res, ref_t arg1, ref_t arg2);
stmt_t addGOTO(sym_t target);
stmt_t addIFGOTO(ref_t arg1, relop_t relop, ref_t arg2, sym_t target);
stmt_t addRETURN(ref_t ret);
stmt_t addREAD(ref_t arg);
stmt_t addWRITE(ref_t arg);

/// ------- Function Table ---------

/**
 * @brief function table entry
 */
typedef struct {
    sym_t funcName; ///< function name
    int paramNum; ///< function parameter number
    operand_t params[MAX_PARAM_NUM]; ///< function parameters
    stmt_t irBegin; ///< the begin statement of this function
    stmt_t irEnd; ///< the end statement of this function
} functable_t;

extern functable_t functab[]; ///< function table

extern int functabSize; ///< number of functions

/**
 * @brief create a new empty function that is not initialized
 * @param funcName function name
 * @return function index
 */
func_t createEmptyFunction(sym_t funcName);

void dumpIR(FILE *fd);

#endif
