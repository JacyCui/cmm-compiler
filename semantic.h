#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdbool.h>

#include "config.h"
#include "symbol.h"

typedef int type_t;
typedef int name_t;
typedef int func_t;

typedef struct typetable_t {
    enum { BASIC, ARRAY, STRUCTURE } kind;
    union {
        enum { INT_TYPE, FLOAT_TYPE } basic;
        struct { 
            type_t base_type;
            int dimension;
            int sizes[MAX_ARRAY_DIMENSION]; 
        } array;
        struct {
            sym_t sym;
            int fieldnum;
            name_t field_names[MAX_STRUCT_FIELDNUM];
        } structure;
    };
} typetable_t;

type_t getIntType();
type_t getFloatType();
type_t addNewEmptyStructureType(sym_t nameSym);
type_t getStructureTypeByName(sym_t nameSym);
type_t addNewEmptyArrayType(type_t type);

bool checkTypeEqual(typetable_t *t1, typetable_t *t2);

typedef struct nametable_t {
    enum { VAR_NAME, FIELD_NAME, STRUCT_NAME } kind;
    sym_t sym;
    type_t type;
} nametable_t;

name_t addVarName(sym_t sym, type_t type);
name_t addFieldName(sym_t sym, type_t type);
name_t getVarName(sym_t sym);
name_t getFieldName(sym_t sym);


typedef struct functable_t {
    sym_t sym;
    type_t ret_type;
    int param_num;
    name_t param_list[MAX_FUNCT_PARAMNUM];
    bool definded;
} functable_t;

func_t addNewEmptyFunction(type_t ret_type, sym_t sym);
func_t getFunctionByName(sym_t sym);

void checkFunctionTable();

extern typetable_t typetab[];
extern nametable_t nametab[];
extern functable_t functab[];

#endif
