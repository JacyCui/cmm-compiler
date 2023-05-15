#ifndef IR_H
#define IR_H

#include "config.h"

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
    enum {
        NONE, ///< no unary operator 
        ADDRESSING, ///< addressing operator
        DEREFERENCE ///< dereference operator
    } uop; ///< unary operator bound to this operand
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
 * @brief statement table index
 */
typedef int stmt_t;

/**
 * @brief type of relational operations
 */
typedef enum {GE, LE, EQ, NE, LT, GT} bop_t;

typedef struct {
    enum {
        LABEL,
        IF_GOTO,
    } kind;
    union {
        sym_t labelName;
    };
} stmtable_t;


/**
 * @brief function table index
 */
typedef int function_t;

/**
 * @brief function table entry
 */
typedef struct {
    sym_t funcName; ///< function name
    int paramNum;
    operand_t params[MAX_PARAM_NUM];
    stmt_t irBegin;
    stmt_t irEnd;
} functable_t;

extern functable_t functab[];
extern int functabSize;

extern int errorno;

#endif
