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

/**
 * @brief dump an operand to stream
 * @param fd output stream
 * @param o operand index
 */
void dumpOpr(FILE* fd, operand_t o);


/// ------- Reference Table ---------

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

/**
 * @brief dump a reference to stream
 * @param fd output stream
 * @param r reference index
 */
void dumpRef(FILE* fd, ref_t r);

/// ------- Statement Table ---------

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
        WRITE, ///< READ y
        NOP ///< an empty statement
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

/**
 * @brief get a label statement by its name
 * 
 * @param labelName name of label
 * @return the label statement, -1 if not found
 */
stmt_t getLabelStmtByName(sym_t labelName);

/**
 * @brief dump statement string representation to stream
 * @param fd output stream
 * @param k statement index
 */
void dumpStmt(FILE *fd, stmt_t k);

/**
 * @brief construct a DEC statement
 * @param array an array operand
 * @return the index of the statement
 */
stmt_t addDEC(operand_t array);

/**
 * @brief construct a CALL statement
 * @param funcName function name
 * @param argsNum number of arguments
 * @param argsBuffer buffer of arguments (in reverse order)
 * @param ret return value
 * @return the index of the statement
 */
stmt_t addCALL(sym_t funcName, int argsNum, ref_t argsBuffer[], ref_t ret);

/**
 * @brief construct a LABEL statement
 * e.g. LABEL x :
 * @param labelName label name
 * @return the index of the statement
 */
stmt_t addLABEL(sym_t labelName);

/**
 * @brief construct a ASSIGN statement
 * e.g. x := y
 * @param res result
 * @param arg argument
 * @return the index of the statement
 */
stmt_t addASSIGN(ref_t res, ref_t arg);

/**
 * @brief construct an ADD statement
 * e.g. x := y + z
 * @param res result
 * @param arg1 the first argument
 * @param arg2 the second argument
 * @return the index of the statement
 */
stmt_t addADD(ref_t res, ref_t arg1, ref_t arg2);

/**
 * @brief construct an SUB statement
 * e.g. x = y - z
 * @param res result
 * @param arg1 the first argument
 * @param arg2 the second argument
 * @return the index of the statement
 */
stmt_t addSUB(ref_t res, ref_t arg1, ref_t arg2);

/**
 * @brief construct an MUL statement
 * e.g. x = y * z
 * @param res result
 * @param arg1 the first argument
 * @param arg2 the second argument
 * @return the index of the statement
 */
stmt_t addMUL(ref_t res, ref_t arg1, ref_t arg2);

/**
 * @brief construct an DIV statement
 * e.g. x = y / z
 * @param res result
 * @param arg1 the first argument
 * @param arg2 the second argument
 * @return the index of the statement
 */
stmt_t addDIV(ref_t res, ref_t arg1, ref_t arg2);

/**
 * @brief construct a GOTO statement
 * e.g. GOTO x
 * @param target the target label
 * @return the index of the statement
 */
stmt_t addGOTO(sym_t target);

/**
 * @brief construct a IFGOTO statement
 * e.g. IF x [relop] y GOTO z
 * @param arg1 the first argument
 * @param relop the relational operator
 * @param arg2 the second argument
 * @param target the target label
 * @return the index of the statement
 */
stmt_t addIFGOTO(ref_t arg1, relop_t relop, ref_t arg2, sym_t target);

/**
 * @brief construct a RETURN statement
 * e.g. RETURN x
 * @param ret the return reference
 * @return the index of the statement
 */
stmt_t addRETURN(ref_t ret);

/**
 * @brief construct a READ statement
 * e.g. READ x
 * @param arg reading argument
 * @return the index of the statement
 */
stmt_t addREAD(ref_t arg);

/**
 * @brief construct a WRITE statement
 * e.g. WRITE x
 * @param arg writing argument
 * @return the index of the statement
 */
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
    bb_t firstBB; ///< the first basic block of this function
    bb_t lastBB; ///< the last basic block of this function
} functable_t;

extern functable_t functab[]; ///< function table

extern int functabSize; ///< number of functions

/**
 * @brief create a new empty function that is not initialized
 * @param funcName function name
 * @return function index
 */
func_t createEmptyFunction(sym_t funcName);

/**
 * @brief dump ir of a function to stream
 * @param fd output stream
 * @param i function index
 */
void dumpFunctionIR(FILE *fd, func_t i);

/**
 * @brief dump string representation of ir into given stream
 * @param fd file stream (e.g. stdout)
 */
void dumpIR(FILE *fd);

#endif
