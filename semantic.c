#include <string.h>
#include <stdio.h>

#include "semantic.h"

typetable_t type_table[MAX_TABLE_LEN] = {
    [0] = { .kind = BASIC, .basic = INT_TYPE},
    [1] = { .kind = BASIC, .basic = FLOAT_TYPE}
};
int type_table_size = 2;

nametable_t name_table[MAX_TABLE_LEN];
int name_table_size = 0;

functable_t func_table[MAX_TABLE_LEN];
int func_table_size = 0;

type_t getIntType() {
    return 0;
}

type_t getFloatType() {
    return 1;
}

typetable_t *getTypeContent(int i) {
    return type_table + i;
}

type_t addNewEmptyStructureType(sym_t nameSym) {
    int i;
    if (nameSym != -1) {
        for (i = 0; i < name_table_size; i++) {
            if (name_table[i].sym == nameSym) {
                return -1;
            }
        }
        name_table[name_table_size].kind = STRUCT_NAME;
        name_table[name_table_size].sym = nameSym;
        name_table[name_table_size].type = type_table_size;
        name_table_size++;
    }
    type_table[type_table_size].kind = STRUCTURE;
    type_table[type_table_size].structure.sym = nameSym;
    type_table[type_table_size].structure.fieldnum = 0;
    return type_table_size++;
}

type_t getStructureTypeByName(sym_t nameSym) {
    int i;
    for (i = 0; i < name_table_size; i++) {
        if (name_table[i].kind == STRUCT_NAME && name_table[i].sym == nameSym) {
            return name_table[i].type;
        }
    }
    return -1;
}

bool checkTypeEqual(typetable_t *t1, typetable_t *t2) {
    if (t1 == t2) {
        return true;
    }
    if (t1->kind != t2->kind) {
        return false;
    }
    switch (t1->kind) {
        case BASIC: return t1->basic == t2->basic;
        case ARRAY: return t1->array.base_type == t2->array.base_type 
                            && t1->array.dimension == t2->array.dimension;
        case STRUCTURE: return t1->structure.sym >= 0 && t2->structure.sym >= 0 
                            && t1->structure.sym == t2->structure.sym;
    }
    return true;
}

type_t addNewEmptyArrayType(type_t type) {
    type_table[type_table_size].kind = ARRAY;
    type_table[type_table_size].array.base_type = type;
    type_table[type_table_size].array.dimension = 0;
    return type_table_size++;
}

name_t addVarName(sym_t sym, type_t type) {
    int i;
    for (i = 0; i < name_table_size; i++) {
        if (name_table[i].sym == sym) {
            return -1;
        }
    }
    name_table[name_table_size].kind = VAR_NAME;
    name_table[name_table_size].sym = sym;
    name_table[name_table_size].type = type;
    return name_table_size++;
}

name_t addFieldName(sym_t sym, type_t type) {
    int i;
    for (i = 0; i < name_table_size; i++) {
        if (name_table[i].sym == sym) {
            return -1;
        }
    }
    name_table[name_table_size].kind = FIELD_NAME;
    name_table[name_table_size].sym = sym;
    name_table[name_table_size].type = type;
    return name_table_size++;
}

name_t getVarName(sym_t sym) {
    int i;
    for (i = 0; i < name_table_size; i++) {
        if (name_table[i].kind == VAR_NAME && name_table[i].sym == sym) {
            return i;
        }
    }
    return -1;
}

name_t getFieldName(sym_t sym) {
    int i;
    for (i = 0; i < name_table_size; i++) {
        if (name_table[i].kind == FIELD_NAME && name_table[i].sym == sym) {
            return i;
        }
    }
    return -1;
}

nametable_t *getNameContent(int i) { 
    return name_table + i;
}

func_t addNewEmptyFunction(type_t ret_type, sym_t nameSym) {
    int i;
    for (i = 0; i < func_table_size; i++) {
        if (func_table[i].sym == nameSym) {
            return -1;
        }
    }
    func_table[func_table_size].ret_type = ret_type;
    func_table[func_table_size].sym = nameSym;
    func_table[func_table_size].param_num = 0;
    return func_table_size++;
}

func_t getFunctionByName(sym_t sym) {
    int i;
    for (i = 0; i < func_table_size; i++) {
        if (func_table[i].sym == sym) {
            return i;
        }
    }
    return -1;
}

functable_t *getFunctContent(func_t i) {
    return func_table + i;
}

void checkFunctionTable() {
    int i;
    for (i = 0; i < func_table_size; i++) {
        if (!func_table[i].definded) {
            printf("Error type 18 at Line %d: Undefined function \"%s\".\n",
                symtab[func_table[i].sym].lineno, symtab[func_table[i].sym].symbol);
        }
    }
}
