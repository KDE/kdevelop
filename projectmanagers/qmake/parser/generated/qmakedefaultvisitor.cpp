// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "qmakedefaultvisitor.h"

namespace QMake
{

void DefaultVisitor::visitArgumentList(ArgumentListAst *node)
{
    if (node->argsSequence)
    {
        const KDevPG::ListNode<ValueAst*> *__it = node->argsSequence->front(), *__end = __it;
        do
        {
            visitNode(__it->element);
            __it = __it->next;
        }
        while (__it != __end);
    }
}

void DefaultVisitor::visitFunctionArguments(FunctionArgumentsAst *node)
{
    visitNode(node->args);
}

void DefaultVisitor::visitItem(ItemAst *node)
{
    visitNode(node->functionArguments);
}

void DefaultVisitor::visitOp(OpAst *)
{
}

void DefaultVisitor::visitOrOperator(OrOperatorAst *node)
{
    if (node->itemSequence)
    {
        const KDevPG::ListNode<ItemAst*> *__it = node->itemSequence->front(), *__end = __it;
        do
        {
            visitNode(__it->element);
            __it = __it->next;
        }
        while (__it != __end);
    }
}

void DefaultVisitor::visitProject(ProjectAst *node)
{
    if (node->statementsSequence)
    {
        const KDevPG::ListNode<StatementAst*> *__it = node->statementsSequence->front(), *__end = __it;
        do
        {
            visitNode(__it->element);
            __it = __it->next;
        }
        while (__it != __end);
    }
}

void DefaultVisitor::visitScope(ScopeAst *node)
{
    visitNode(node->functionArguments);
    visitNode(node->scopeBody);
    visitNode(node->orOperator);
}

void DefaultVisitor::visitScopeBody(ScopeBodyAst *node)
{
    if (node->statementsSequence)
    {
        const KDevPG::ListNode<StatementAst*> *__it = node->statementsSequence->front(), *__end = __it;
        do
        {
            visitNode(__it->element);
            __it = __it->next;
        }
        while (__it != __end);
    }
}

void DefaultVisitor::visitStatement(StatementAst *node)
{
    visitNode(node->var);
    visitNode(node->scope);
}

void DefaultVisitor::visitValue(ValueAst *)
{
}

void DefaultVisitor::visitValueList(ValueListAst *node)
{
    if (node->listSequence)
    {
        const KDevPG::ListNode<ValueAst*> *__it = node->listSequence->front(), *__end = __it;
        do
        {
            visitNode(__it->element);
            __it = __it->next;
        }
        while (__it != __end);
    }
}

void DefaultVisitor::visitVariableAssignment(VariableAssignmentAst *node)
{
    visitNode(node->op);
    visitNode(node->values);
}


} // end of namespace QMake

