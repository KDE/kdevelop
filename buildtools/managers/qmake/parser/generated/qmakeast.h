// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef QMAKE_AST_H_INCLUDED
#define QMAKE_AST_H_INCLUDED

#include <QtCore/QList>
#include <kdev-pg-list.h>



#include <QtCore/QString>

namespace QMake
{
class Lexer;
}


namespace QMake
{

struct ArgumentListAst;
struct FunctionArgumentsAst;
struct ItemAst;
struct OpAst;
struct OrOperatorAst;
struct ProjectAst;
struct ScopeAst;
struct ScopeBodyAst;
struct StatementAst;
struct ValueAst;
struct ValueListAst;
struct VariableAssignmentAst;


struct  AstNode
{
    enum AstNodeKind
    {
        ArgumentListKind = 1000,
        FunctionArgumentsKind = 1001,
        ItemKind = 1002,
        OpKind = 1003,
        OrOperatorKind = 1004,
        ProjectKind = 1005,
        ScopeKind = 1006,
        ScopeBodyKind = 1007,
        StatementKind = 1008,
        ValueKind = 1009,
        ValueListKind = 1010,
        VariableAssignmentKind = 1011,
        AST_NODE_KIND_COUNT
    };

    int kind;
    qint64 startToken;
    qint64 endToken;
};

struct  ArgumentListAst: public AstNode
{
    enum { KIND = ArgumentListKind };

    const KDevPG::ListNode<ValueAst *> *argsSequence;
};

struct  FunctionArgumentsAst: public AstNode
{
    enum { KIND = FunctionArgumentsKind };

    ArgumentListAst *args;
};

struct  ItemAst: public AstNode
{
    enum { KIND = ItemKind };

    qint64 id;
    FunctionArgumentsAst *functionArguments;
};

struct  OpAst: public AstNode
{
    enum { KIND = OpKind };

    qint64 optoken;
};

struct  OrOperatorAst: public AstNode
{
    enum { KIND = OrOperatorKind };

    const KDevPG::ListNode<ItemAst *> *itemSequence;
};

struct  ProjectAst: public AstNode
{
    enum { KIND = ProjectKind };

    const KDevPG::ListNode<StatementAst *> *statementsSequence;
};

struct  ScopeAst: public AstNode
{
    enum { KIND = ScopeKind };

    FunctionArgumentsAst *functionArguments;
    ScopeBodyAst *scopeBody;
    OrOperatorAst *orOperator;
};

struct  ScopeBodyAst: public AstNode
{
    enum { KIND = ScopeBodyKind };

    const KDevPG::ListNode<StatementAst *> *statementsSequence;
};

struct  StatementAst: public AstNode
{
    enum { KIND = StatementKind };

    bool isNewline;
    bool isExclam;
    qint64 id;
    VariableAssignmentAst *var;
    ScopeAst *scope;
};

struct  ValueAst: public AstNode
{
    enum { KIND = ValueKind };

    qint64 value;
};

struct  ValueListAst: public AstNode
{
    enum { KIND = ValueListKind };

    const KDevPG::ListNode<ValueAst *> *listSequence;
};

struct  VariableAssignmentAst: public AstNode
{
    enum { KIND = VariableAssignmentKind };

    OpAst *op;
    ValueListAst *values;
};



} // end of namespace QMake

#endif

