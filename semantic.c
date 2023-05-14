#include <string.h>
#include <stdio.h>

#include "semantic.h"

typetable_t typetab[MAX_TABLE_LEN] = {
    [0] = { .kind = BASIC, .basic = INT_TYPE },
    [1] = { .kind = BASIC, .basic = FLOAT_TYPE }
};
type_t typetab_size = 2;

nametable_t nametab[MAX_TABLE_LEN] = {
    [0] = { .kind = VAR_NAME, .sym = -1, .type = 0 }
};
name_t name_table_size = 1;

functable_t functab[MAX_TABLE_LEN] = {
    [0] = { .sym = SYMBOL_READ, .param_num = 0, .ret_type = 0, .definded = true },
    [1] = { .sym = SYMBOL_WRITE, .param_num = 1, .ret_type = 0, 
            .definded = true, .param_list = {0} }
};
func_t func_table_size = 2;

type_t getIntType() {
    return 0;
}

type_t getFloatType() {
    return 1;
}

type_t addNewEmptyStructureType(sym_t nameSym) {
    int i;
    if (nameSym != -1) {
        for (i = 0; i < name_table_size; i++) {
            if (nametab[i].sym == nameSym) {
                return -1;
            }
        }
        nametab[name_table_size].kind = STRUCT_NAME;
        nametab[name_table_size].sym = nameSym;
        nametab[name_table_size].type = typetab_size;
        name_table_size++;
    }
    typetab[typetab_size].kind = STRUCTURE;
    typetab[typetab_size].structure.sym = nameSym;
    typetab[typetab_size].structure.fieldnum = 0;
    return typetab_size++;
}

type_t getStructureTypeByName(sym_t nameSym) {
    int i;
    for (i = 0; i < name_table_size; i++) {
        if (nametab[i].kind == STRUCT_NAME && nametab[i].sym == nameSym) {
            return nametab[i].type;
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
    typetab[typetab_size].kind = ARRAY;
    typetab[typetab_size].array.base_type = type;
    typetab[typetab_size].array.dimension = 0;
    return typetab_size++;
}

name_t addVarName(sym_t sym, type_t type) {
    int i;
    for (i = 0; i < name_table_size; i++) {
        if (nametab[i].sym == sym) {
            return -1;
        }
    }
    nametab[name_table_size].kind = VAR_NAME;
    nametab[name_table_size].sym = sym;
    nametab[name_table_size].type = type;
    return name_table_size++;
}

name_t addFieldName(sym_t sym, type_t type) {
    int i;
    for (i = 0; i < name_table_size; i++) {
        if (nametab[i].sym == sym) {
            return -1;
        }
    }
    nametab[name_table_size].kind = FIELD_NAME;
    nametab[name_table_size].sym = sym;
    nametab[name_table_size].type = type;
    return name_table_size++;
}

name_t getVarName(sym_t sym) {
    int i;
    for (i = 0; i < name_table_size; i++) {
        if (nametab[i].kind == VAR_NAME && nametab[i].sym == sym) {
            return i;
        }
    }
    return -1;
}

name_t getFieldName(sym_t sym) {
    int i;
    for (i = 0; i < name_table_size; i++) {
        if (nametab[i].kind == FIELD_NAME && nametab[i].sym == sym) {
            return i;
        }
    }
    return -1;
}

func_t addNewEmptyFunction(type_t ret_type, sym_t nameSym) {
    int i;
    for (i = 0; i < func_table_size; i++) {
        if (functab[i].sym == nameSym) {
            return -1;
        }
    }
    functab[func_table_size].ret_type = ret_type;
    functab[func_table_size].sym = nameSym;
    functab[func_table_size].param_num = 0;
    return func_table_size++;
}

func_t getFunctionByName(sym_t sym) {
    int i;
    for (i = 0; i < func_table_size; i++) {
        if (functab[i].sym == sym) {
            return i;
        }
    }
    return -1;
}

void checkFunctionTable() {
    int i;
    for (i = 0; i < func_table_size; i++) {
        if (!functab[i].definded) {
            printf("Error type 18 at Line %d: Undefined function \"%s\".\n",
                symtab[functab[i].sym].lineno, symtab[functab[i].sym].symbol);
        }
    }
}
