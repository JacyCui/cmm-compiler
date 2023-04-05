#include <stdio.h>
#include <assert.h>

#include "ast.h"
#include "syntax.tab.h"
#include "symbol.h"
#include "checker.h"
#include "semantic.h"

static type_t processSpecifier(astnode_t *p);

static void processExtDef(astnode_t *p);
static void processDef(astnode_t *p, typetable_t *structure);
static sym_t processArrayDec(type_t type, astnode_t *p, typetable_t *structure, functable_t *function);
static void processVarDec(type_t type, astnode_t *id, typetable_t *structure, functable_t *function);
static void processDec(type_t type, astnode_t *p, typetable_t *structure);

static type_t processExpr(astnode_t *p);

static func_t processFunDec(type_t ret_type, astnode_t *p, bool is_def);
static void processVarList(astnode_t *p, functable_t *function);
static void processParamDec(astnode_t *p, functable_t *function);
static func_t curFunc;

void check(astnode_t *p) {
    if (p == NULL) {
        return;
    }
    switch (p->type) {
        case ExtDef: processExtDef(p); return;
        case Def: processDef(p, NULL); return;
        case Exp: processExpr(p); return;
    }
    if (p->type == Stmt && p->childs[0]->type == RETURN) {
        if (processExpr(p->childs[1]) != getFunctContent(curFunc)->ret_type) {
            printf("Error type 8 at Line %d: Type mismatched for return.\n", p->lineno);
        }
        return;
    }
    int i;
    for (i = 0; i < p->child_num; i++) {
        check(p->childs[i]);
    }
}

static void processExtDef(astnode_t *p) {
    assert(p->type == ExtDef);
    type_t type = processSpecifier(p->childs[0]);
    if (type == -1) {
        return;
    }
    if (p->child_num == 2) {
        return;
    }
    astnode_t *ext_dec_list;
    astnode_t *var_dec;
    if (p->childs[1]->type == ExtDecList) {
        while (ext_dec_list->child_num == 3) {
            var_dec = ext_dec_list->childs[0];
            if (var_dec->childs[0]->type == ID) {
                processVarDec(type, var_dec->childs[0], NULL, NULL);
            } else {
                processArrayDec(type, var_dec, NULL, NULL);
            }
            ext_dec_list = ext_dec_list->childs[2];
        }
        assert(ext_dec_list->child_num == 1);
        var_dec = ext_dec_list->childs[0];
        if (var_dec->childs[0]->type == ID) {
            processVarDec(type, var_dec->childs[0], NULL, NULL);
        } else {
            processArrayDec(type, var_dec, NULL, NULL);
        }
        return;
    }
    bool is_def = p->childs[2]->type == CompSt;
    func_t func = processFunDec(type, p->childs[1], is_def);
    if (p->childs[2]->type == CompSt) {
        curFunc = func;
        check(p->childs[2]);
    }
}

static void processParamDec(astnode_t *p, functable_t *function) {
    assert(p->type == ParamDec);
    type_t type = processSpecifier(p->childs[0]);
    astnode_t *var_dec = p->childs[1];
    if (var_dec->childs[0]->type == ID) {
        processVarDec(type, var_dec->childs[0], NULL, function);
        return;
    }
    processArrayDec(type, var_dec, NULL, function);
}

static void processVarList(astnode_t *p, functable_t *function) {
    assert(p->type == VarList);
    while (p->child_num == 3) {
        processParamDec(p->childs[0], function);
        p = p->childs[2];
    }
    assert(p->child_num == 1);
    processParamDec(p->childs[0], function);
}

static void getParamType(astnode_t *p, typetable_t *param_type) {
    assert(p->type == ParamDec);
    type_t base = processSpecifier(p->childs[0]);
    typetable_t *base_type = getTypeContent(base);
    astnode_t *var_dec = p->childs[1];
    if (var_dec->child_num == 1) {
        *param_type = *base_type;
        return ;
    }
    int count = 1;
    while (var_dec->child_num == 4) {
        count++;
        var_dec = var_dec->childs[0];
    }
    param_type->kind = ARRAY;
    param_type->array.base_type = base;
    param_type->array.dimension = count;
}

static bool checkFuncSig(type_t type, astnode_t *p, functable_t *function) {
    assert(p->type == FunDec);
    if (p->child_num == 3) {
        return type == function->ret_type && function->param_num == 0;
    }
    p = p->childs[2];
    assert(p->type == VarList);
    int i;
    nametable_t *param_name;
    typetable_t param_type;
    for (i = 0; i < function->param_num; i++) {
        param_name = getNameContent(function->param_list[i]);
        getParamType(p->childs[0], &param_type);
        if (!checkTypeEqual(getTypeContent(param_name->type), &param_type)) {
            return false;
        }
        if (p->child_num == 1) {
            break;
        }
        p = p->childs[2];
    }
    if (p->child_num != 1 || i != function->param_num - 1) {
        return false;
    }
    return true;
}

static func_t processFunDec(type_t ret_type, astnode_t *p, bool is_def) {
    assert(p->type == FunDec);
    astnode_t *id = p->childs[0];
    func_t fun = addNewEmptyFunction(ret_type, id->tabid);
    functable_t *function;
    if (fun != -1) {
        function = getFunctContent(fun);
        function->definded = is_def;
        if (p->child_num == 4) {
            processVarList(p->childs[2], function);
        }
        return fun;
    }
    fun = getFunctionByName(id->tabid);
    function = getFunctContent(fun);
    if (function->definded && is_def) {
        printf("Error type 4 at Line %d: Redefined function \"%s\".\n",
            id->lineno, symtab[id->tabid].symbol);
        return fun;
    }
    function->definded = function->definded || is_def;
    if (!checkFuncSig(ret_type, p, function)) {
        // 错误类型19:函数的多次声明互相冲突(即函数名一致，但返回类型、形参数量
        // 或者形参类型不一致)，或者声明与定义之间互相冲突。
        printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n",
            id->lineno, symtab[id->tabid].symbol);    
    }
    return fun;
}

static type_t processSpecifier(astnode_t *p) {
    astnode_t *p0 = p->childs[0];
    if (p0->type == TYPE) {
        if (p0->tabid == INT) {
            return getIntType();
        } else {
            return getFloatType();
        }
    }
    assert(p0->type == StructSpecifier);
    sym_t struct_sym;
    astnode_t *tag = p0->childs[1];
    type_t new_type;
    typetable_t *type;
    astnode_t *def_list;
    if (p0->child_num == 5) {
        if (tag != NULL) {
            assert(tag->childs[0]->type == ID);
            struct_sym = tag->childs[0]->tabid;
        } else {
            struct_sym = -1;
        }
        new_type = addNewEmptyStructureType(struct_sym);
        if (new_type == -1) {
            // 错误类型16:结构体的名字与前面定义过的结构体或变量的名字重复。
            printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", 
                tag->lineno, symtab[struct_sym].symbol);
            return -1;
        }
        type = getTypeContent(new_type);
        def_list = p0->childs[3];
        while (def_list != NULL) {
            processDef(def_list->childs[0], type);
            def_list = def_list->childs[1];
        }
        return new_type;
    }
    assert(p0->child_num == 2);
    assert(tag->childs[0]->type == ID);
    struct_sym = tag->childs[0]->tabid;
    new_type = getStructureTypeByName(struct_sym);
    if (new_type == -1) {
        // 错误类型17: 使用未定义过的结构体
        printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",
            tag->lineno, symtab[struct_sym].symbol);
    }
    return new_type;
}

static void processDef(astnode_t *p, typetable_t *structure) {
    assert(p->type == Def);
    type_t type = processSpecifier(p->childs[0]);
    if (type == -1) return;
    astnode_t *dec_list = p->childs[1];
    assert(dec_list->type == DecList);
    while (dec_list->child_num == 3) {
        processDec(type, dec_list->childs[0], structure);
        dec_list = dec_list->childs[2];
    }
    assert(dec_list->child_num == 1);
    processDec(type, dec_list->childs[0], structure);
}

static sym_t processArrayDec(type_t type, astnode_t *p, typetable_t *structure, functable_t *function) {
    assert(p->type == VarDec);
    type_t new_type = addNewEmptyArrayType(type);
    typetable_t *array = getTypeContent(new_type);
    astnode_t *int_node;
    while (p->child_num == 4) {
        int_node = p->childs[2];
        assert(int_node->type == INT);
        array->array.sizes[array->array.dimension++] = intab[int_node->tabid].value;
        p = p->childs[0];
    }
    assert(p->child_num == 1);
    astnode_t *id = p->childs[0];
    assert(id->type == ID);
    name_t new_name;
    if (structure == NULL) {
        new_name = addVarName(id->tabid, new_type);
    } else {
        new_name = addFieldName(id->tabid, new_type);
    }
    if (new_name == -1) {
        if (structure == NULL) {
            // 错误类型3: 变量出现重复定义，或变量与前面定义过的结构体名字重复。
            printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", 
                id->lineno, symtab[id->tabid].symbol);
        } else {
            // 错误类型15: 结构体中域名重复定义
            printf("Error type 15 at Line %d: Redefined field \"%s\".\n",
                id->lineno, symtab[id->tabid].symbol);
        }
        return -1;
    }
    if (structure != NULL) {
        structure->structure.field_names[structure->structure.fieldnum++] = new_name;
    }
    if (function != NULL) {
        function->param_list[function->param_num++] = new_name;
    }
    return id->tabid;
}

static void processVarDec(type_t type, astnode_t *id, typetable_t *structure, functable_t *function) {
    assert(id->type == ID);
    name_t new_name;
    if (structure == NULL) {
        new_name = addVarName(id->tabid, type);
    } else {
        new_name = addFieldName(id->tabid, type);
    }
    if (new_name == -1) {
        if (structure == NULL) {
            // 错误类型3: 变量出现重复定义，或变量与前面定义过的结构体名字重复。
            printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", 
                id->lineno, symtab[id->tabid].symbol);
        } else {
            // 错误类型15: 结构体中域名重复定义
            printf("Error type 15 at Line %d: Redefined field \"%s\".\n",
                id->lineno, symtab[id->tabid].symbol);
        }
    } else {
        if (structure != NULL) {
            structure->structure.field_names[structure->structure.fieldnum++] = new_name;
        }
        if (function != NULL) {
            function->param_list[function->param_num++] = new_name;
        }
    }
}

static void processDec(type_t type, astnode_t *p, typetable_t *structure) {
    assert(p->type == Dec);
    astnode_t *var_dec = p->childs[0];
    type_t init_type;
    astnode_t *id;
    if (var_dec->childs[0]->type == ID) {
        id = var_dec->childs[0];
        processVarDec(type, id, structure, NULL);
        if (p->child_num == 3) {
            if (structure != NULL) {
                // 错误类型15: 在定义时对域进行初始化
                printf("Error type 15 at Line %d: Initialize field \"%s\".\n",
                    p->childs[1]->lineno, symtab[id->tabid].symbol);
            } else {
                assert(p->childs[2]->type == Exp);
                init_type = processExpr(p->childs[2]);
                if (init_type != -1 && type != init_type) {
                    // 错误类型5: 赋值号两边的表达式类型不匹配
                    printf("Error type 5 at Line %d: Type mismatched for assignment.\n",
                        p->childs[1]->lineno);
                }
            }
        }
        return;
    }
    sym_t ary_sym = processArrayDec(type, var_dec, structure, NULL);
    if (p->child_num == 3) {
        if (structure != NULL) {
            if (ary_sym != -1) {
                // 错误类型15: 在定义时对域进行初始化
                printf("Error type 15 at Line %d: Initialize field \"%s\".\n",
                    p->childs[1]->lineno, symtab[ary_sym].symbol);
            }
        } else {
            // 错误类型5: 赋值号两边的表达式类型不匹配
            printf("Error type 5 at Line %d: Type mismatched for assignment.\n",
                p->childs[1]->lineno);
        }
    }
}

static type_t processArray(astnode_t *p, int dim) {
    assert(p->type == Exp);
    type_t index;
    if (p->child_num == 4 && p->childs[1]->type == LB) {
        index = processExpr(p->childs[2]);
        if (type_table[index].kind != BASIC || type_table[index].basic != INT_TYPE) {
            printf("Error type 12 at Line %d: index is not an integer.\n", p->childs[2]->lineno);
        }
        return processArray(p->childs[0], dim + 1);
    }
    type_t type;
    type = processExpr(p);
    if (type_table[type].kind == ARRAY && type_table[type].array.dimension == dim) {
        return type_table[type].array.base_type;
    }
    printf("Error type 10 at Line %d: This is not a proper array.\n", p->lineno);
    return -1;
}

static type_t checkFuncArgs(astnode_t *p, functable_t *function) {
    if (p->type == RP) {
        if (function->param_num != 0) {
            printf("Error type 9 at Line %d: Function \"%s\" has wrong arguments.\n",
                p->lineno, symtab[function->sym].symbol);   
        }
        return function->ret_type;
    }
    int i;
    type_t param_type;
    for (i = 0; i < function->param_num; i++) {
        param_type = name_table[function->param_list[i]].type;
        if (param_type != processExpr(p->childs[0])) {
            printf("Error type 9 at Line %d: Function \"%s\" has wrong arguments.\n",
                p->lineno, symtab[function->sym].symbol);   
            return function->ret_type;
        }
        if (p->child_num == 1) {
            break;
        }
    }
    if (p->child_num != 1 || i != function->param_num - 1) {
        printf("Error type 9 at Line %d: Function \"%s\" has wrong arguments.\n",
                p->lineno, symtab[function->sym].symbol);  
    }
    return function->ret_type;
}

static type_t processExpr(astnode_t *p) {
    assert(p->type == Exp);
    astnode_t *atom = p->childs[0];
    type_t type;
    name_t name;
    if (p->child_num == 1) {
        switch (atom->type) {
            case INT: return getIntType();
            case FLOAT: return getFloatType();
            case ID: 
                name = getVarName(atom->tabid);
                if (name == -1) {
                    printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",
                        atom->lineno, symtab[atom->tabid].symbol);
                    return -1;
                }
                return name_table[name].type;
        }
        return -1;
    }
    if (atom->type == LP) {
        return processExpr(p->childs[1]);
    }
    if (atom->type == MINUS) {
        type = processExpr(p->childs[1]);
        if (type == getIntType() || type == getFloatType()) {
            return type;
        } else {
            printf("Error type 7 at Line %d: Type mismatched for operands.\n", atom->lineno);
            return -1;
        }
    }
    if (atom->type == NOT) {
        type = processExpr(p->childs[1]);
        if (type == getIntType()) {
            return type;
        } else {
            printf("Error type 7 at Line %d: Type mismatched for operands.\n", atom->lineno);
            return getIntType();
        }
    }
    func_t fun;
    if (atom->type == ID) {
        if (getVarName(atom->tabid) != -1 || getFieldName(atom->tabid) != -1) {
            printf("Error type 11 at Line %d: \"%s\" is not a function.\n", 
                atom->lineno, symtab[atom->tabid].symbol);
            return -1;
        }
        fun = getFunctionByName(atom->tabid);
        if (fun == -1) {
            printf("Error type 2 at Line %d: Undefined function \"%s\".\n",
                atom->lineno, symtab[atom->tabid].symbol);
            return -1;
        }
        return checkFuncArgs(p->childs[2], func_table + fun);
    }
    name_t field;
    int i;
    if (p->childs[1]->type == DOT) {
        type = processExpr(atom);
        if (type_table[type].kind != STRUCTURE) {
            printf("Error type 13 at Line %d: Illegal use of \".\".\n", p->childs[1]->lineno);
            return -1;
        }
        field = getFieldName(p->childs[2]->tabid);
        if (field == -1) {
            printf("Error type 14 at Line %d: Non-existent field \"%s\".\n",
                p->childs[2]->lineno, symtab[p->childs[2]->tabid].symbol);
            return -1;
        }
        for (i = 0; i < type_table[type].structure.fieldnum; i++) {
            if (type_table[type].structure.field_names[i] == field) {
                return name_table[field].type;
            }
        }
        printf("Error type 14 at Line %d: Non-existent field \"%s\".\n",
                p->childs[2]->lineno, symtab[p->childs[2]->tabid].symbol);
        return -1;
    }
    if (p->childs[1]->type == LB) {
        return processArray(p, 0);
    }
    assert(p->child_num == 3);
    astnode_t *op = p->childs[1];
    type_t type1 = processExpr(p->childs[0]);
    type_t type2 = processExpr(p->childs[2]);
    if (op->type == ASSIGNOP) {
        if ((atom->child_num == 1 && atom->childs[0]->type == ID) 
            || (atom->child_num == 4 && atom->childs[1]->type == LB)
            || (atom->child_num == 3 && atom->childs[1]->type == DOT)) {
            if (type1 == -1 || type2 == -1 || type1 == type2) {
                return type1 != -1 ? type1 : type2;
            }
            printf("Error type 5 at Line %d: Type mismatched for assignment.\n", op->lineno);
            return -1;
        }
        printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",
            op->lineno);
        return -1;
    }
    if (type1 != -1 && type2 != -1 && type1 != type2) {
        printf("Error type 7 at Line %d: Type mismatched for operands.\n", op->lineno);
        return -1;
    }
    if (op->type == RELOP) {
        return getIntType();
    }
    if (op->type == AND || op->type == OR) {
        if ((type1 != -1 && type1 != getIntType()) 
            || (type2 != -1 && type2 != getIntType())) {
            printf("Error type 7 at Line %d: Type mismatched for operands.\n", op->lineno);
            return -1;
        }
        return type1 != -1 ? type1 : type2;
    }
    return type1 != -1 ? type1 : type2;
}

