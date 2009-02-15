// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef QMAKE_DEFAULT_VISITOR_H_INCLUDED
#define QMAKE_DEFAULT_VISITOR_H_INCLUDED

#include "qmakevisitor.h"

namespace QMake
{

class  DefaultVisitor: public Visitor
{
public:
    virtual void visitArgumentList(ArgumentListAst *node);
    virtual void visitFunctionArguments(FunctionArgumentsAst *node);
    virtual void visitItem(ItemAst *node);
    virtual void visitOp(OpAst *node);
    virtual void visitOrOperator(OrOperatorAst *node);
    virtual void visitProject(ProjectAst *node);
    virtual void visitScope(ScopeAst *node);
    virtual void visitScopeBody(ScopeBodyAst *node);
    virtual void visitStatement(StatementAst *node);
    virtual void visitValue(ValueAst *node);
    virtual void visitValueList(ValueListAst *node);
    virtual void visitVariableAssignment(VariableAssignmentAst *node);
};

} // end of namespace QMake

#endif

