// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef QMAKE_VISITOR_H_INCLUDED
#define QMAKE_VISITOR_H_INCLUDED

#include "qmakeast.h"

namespace QMake
{

class  Visitor
{
    typedef void (Visitor::*ParserFuncType)(AstNode *);
    static ParserFuncType sParserTable[];

public:
    virtual ~Visitor() {}
    virtual void visitNode(AstNode *node)
    {
        if (node) (this->*sParserTable[node->kind - 1000])(node);
    }
    virtual void visitArgumentList(ArgumentListAst *) {}
    virtual void visitFunctionArguments(FunctionArgumentsAst *) {}
    virtual void visitItem(ItemAst *) {}
    virtual void visitOp(OpAst *) {}
    virtual void visitOrOperator(OrOperatorAst *) {}
    virtual void visitProject(ProjectAst *) {}
    virtual void visitScope(ScopeAst *) {}
    virtual void visitScopeBody(ScopeBodyAst *) {}
    virtual void visitStatement(StatementAst *) {}
    virtual void visitValue(ValueAst *) {}
    virtual void visitValueList(ValueListAst *) {}
    virtual void visitVariableAssignment(VariableAssignmentAst *) {}
};

} // end of namespace QMake

#endif

