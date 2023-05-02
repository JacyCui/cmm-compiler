#include <assert.h>
#include <stdio.h>

#include "syntax.tab.h"
#include "irgen.h"

static void translateFunDec(astnode_t *p) {
    assert(p->type == FunDec);
    astnode_t *id = p->childs[0];
    sym_t funcName = id->tabid;
    func_t func = getFunctionByName(funcName);
    assert(func != -1);
    assert(functab[func].definded);
    IR_FUNCTION(funcName); // Function funcName :
    operand_t param;
    int i;
    for (i = 0; i < functab[func].param_num; i++) {
        param = getOrCreateVarByName(functab[func].param_list[i]);
        IR_PARAM(param); // PARAM param
    }
}

static void translateCond(astnode_t *p, label_t trueLabel, label_t falseLabel);

static void translateExp(astnode_t *p, operand_t place) {
    assert(p->type == Exp);

    int value;
    if (p->child_num == 1 && p->childs[0]->type == INT) {
        value = intab[p->childs[0]->tabid].value;
        IR_ASSIGN(place, getOrCreateConstant(value));
        return;
    }

    operand_t variable;
    if (p->child_num == 1 && p->childs[0]->type == ID) {
        variable = getOrCreateVarByName(p->childs[0]->tabid);
        IR_ASSIGN(place, variable);
        return;
    }

    operand_t t1;
    astnode_t* exp1, *exp2;
    if (p->child_num == 3 && p->childs[1]->type == ASSIGNOP) {
        exp1 = p->childs[0];
        exp2 = p->childs[2];
        assert(exp1->type == Exp);
        assert(exp1->child_num == 1 && exp1->childs[0]->type == ID);
        variable = getOrCreateVarByName(getVarName(exp1->childs[0]->tabid));
        t1 = newTemp();
        translateExp(exp2, t1);
        IR_ASSIGN(variable, t1);
        IR_ASSIGN(place, variable);
        return;
    }

    operand_t t2;
    if (p->child_num == 3 && p->childs[1]->type == PLUS) {
        exp1 = p->childs[0];
        exp2 = p->childs[2];
        t1 = newTemp();
        t2 = newTemp();
        translateExp(exp1, t1);
        translateExp(exp2, t2);
        IR_ADD(place, t1, t2);
        return;
    }

    if (p->child_num == 3 && p->childs[1]->type == MINUS) {
        exp1 = p->childs[0];
        exp2 = p->childs[2];
        t1 = newTemp();
        t2 = newTemp();
        translateExp(exp1, t1);
        translateExp(exp2, t2);
        IR_SUB(place, t1, t2);
        return;
    }

    if (p->child_num == 3 && p->childs[1]->type == STAR) {
        exp1 = p->childs[0];
        exp2 = p->childs[2];
        t1 = newTemp();
        t2 = newTemp();
        translateExp(exp1, t1);
        translateExp(exp2, t2);
        IR_MUL(place, t1, t2);
        return;
    }

    if (p->child_num == 3 && p->childs[1]->type == DIV) {
        exp1 = p->childs[0];
        exp2 = p->childs[2];
        t1 = newTemp();
        t2 = newTemp();
        translateExp(exp1, t1);
        translateExp(exp2, t2);
        IR_DIV(place, t1, t2);
        return;
    }

    if (p->child_num == 2 && p->childs[0]->type == MINUS) {
        exp1 = p->childs[1];
        t1 = newTemp();
        translateExp(exp1, t1);
        IR_SUB(place, getOrCreateConstant(0), t1);
        return;
    }

    if (p->child_num == 3 && p->childs[0]->type == LP) {
        exp1 = p->childs[1];
        translateExp(exp1, place);
        return;
    }

    label_t label1, label2;
    if ((
            p->child_num == 3 && (
            p->childs[1]->type == RELOP || 
            p->childs[1]->type == AND || 
            p->childs[1]->type == OR)
        ) 
        ||
        (p->child_num == 2 && p->childs[0]->type == NOT)) {
        label1 = newLabel();
        label2 = newLabel();
        IR_ASSIGN(place, getOrCreateConstant(0));
        translateCond(p, label1, label2);
        IR_LABEL(label1);
        IR_ASSIGN(place, getOrCreateConstant(1));
        IR_LABEL(label2);
        return;
    }

    // TODO: process function call and array indexing

    printf("Cannot translate: Code contains variables or parameters of structure or float type.");
    exit(0);
}

static void translateCond(astnode_t *p, label_t trueLabel, label_t falseLabel) {
    assert(p->type == Exp);
    
    if (p->child_num == 3 && p->childs[1]->type == RELOP) {

    }

    

}

static void translateDec(astnode_t *p) {
    assert(p->type == Dec);
    astnode_t *varDec = p->childs[0];
    assert(varDec->type == VarDec);

    sym_t varSym;
    while (varDec->child_num != 1) {
        varDec = varDec->childs[0];
    }
    assert(varDec->child_num == 1);
    assert(varDec->childs[0]->type == ID);
    varSym = varDec->childs[0]->tabid;

    name_t varName = getVarName(varSym);
    operand_t place = getOrCreateVarByName(varName);

    typetable_t* varType = typetab + nametab[varName].type;
    int i;
    int size = 1;
    if (varType->kind == ARRAY) {
        for (i = 0; i < varType->array.dimension; i++) {
            size *= varType->array.sizes[i];
        }
        IR_DEC(place, size * 4);
    }

    if (p->child_num == 1) {
        return;
    }
    
    assert(p->child_num == 3);
    astnode_t *exp = p->childs[2];
    translateExp(exp, place);
}

static void translateDefList(astnode_t *p) {
    assert(p->type == DefList);
    astnode_t* def;
    astnode_t* decList;
    astnode_t* dec;
    while (p) {
        def = p->childs[0];
        assert(def->type == Def);
        decList = def->childs[1];
        while (decList->child_num == 3) {
            dec = decList->childs[0];
            translateDec(dec);
            decList = decList->childs[2];
        }
        assert(decList->child_num == 1);
        dec = decList->childs[0];
        translateDec(dec);
        p = p->childs[1];
    }
}

static void translateCompSt(astnode_t *p) {
    assert(p->type == CompSt);

    astnode_t* defList = p->childs[1];
    if (defList) {
        translateDefList(defList);
    }

    astnode_t* stmtList = p->childs[2];
    astnode_t* stmt;
    while (stmtList) {
        stmt = stmtList->childs[0];
        // TODO: translate stmt
        stmtList = stmtList->childs[1];
    }

}

void genIR(astnode_t *p) {
    assert(p->type == Program);
    astnode_t *extDefList = p->childs[0];
    astnode_t *extDef;
    while (extDefList) {
        assert(extDefList->type == ExtDefList);
        extDef = extDefList->childs[0];
        
        assert(extDef->type == ExtDef);
        if (extDef->child_num == 3 && 
            extDef->childs[1]->type == FunDec && 
            extDef->childs[2]->type == CompSt) {
            translateFunDec(extDef->childs[1]);
            translateCompSt(extDef->childs[2]);
        }

        extDefList = extDefList->childs[1];
    }
}

