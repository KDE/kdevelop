/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMAKEAST_H
#define QMAKEAST_H

#include <QString>
#include <QList>

#include "parser_export.h"

namespace KDevelop
{
class DUContext;
}

namespace QMake
{
class ValueAST;

class KDEVQMAKEPARSER_EXPORT AST
{
public:

    enum Type
    {
        Project = 0,
        ScopeBody = 1,
        Assignment = 2,
        FunctionCall = 3,
        SimpleScope = 4,
        Or = 5,
        Value = 6,
        Invalid = 7
    };

    AST( AST* parent, AST::Type type );
    virtual ~AST();
    AST::Type type;
    int startLine;
    int endLine;
    int startColumn;
    int endColumn;
    int start;
    int end;
    AST* parent;
    KDevelop::DUContext* context;
};

class KDEVQMAKEPARSER_EXPORT StatementAST : public AST
{
public:
    StatementAST( AST* parent, AST::Type type );
    ~StatementAST() override;
};

class KDEVQMAKEPARSER_EXPORT ScopeBodyAST: public AST
{
public:
    explicit ScopeBodyAST( AST* parent, AST::Type type = AST::ScopeBody );
    ~ScopeBodyAST() override;
    QList<StatementAST*> statements;
};

class KDEVQMAKEPARSER_EXPORT ProjectAST : public ScopeBodyAST
{
public:
    explicit ProjectAST();
    ~ProjectAST() override;
    QString filename;


};

class KDEVQMAKEPARSER_EXPORT AssignmentAST : public StatementAST
{
public:
    explicit AssignmentAST( AST* parent );
    ~AssignmentAST() override;

    ValueAST* identifier;
    ValueAST* op;
    QList<ValueAST*> values;
};


class KDEVQMAKEPARSER_EXPORT ScopeAST : public StatementAST
{
public:
    explicit ScopeAST( AST* parent, AST::Type type);
    ~ScopeAST() override;
    ScopeBodyAST* body;
};

class KDEVQMAKEPARSER_EXPORT FunctionCallAST : public ScopeAST
{
public:
    explicit FunctionCallAST( AST* parent );
    ~FunctionCallAST() override;
    ValueAST* identifier;
    QList<ValueAST*> args;
};


class KDEVQMAKEPARSER_EXPORT SimpleScopeAST : public ScopeAST
{
public:
    explicit SimpleScopeAST( AST* parent );
    ~SimpleScopeAST() override;
    ValueAST* identifier;
};

class KDEVQMAKEPARSER_EXPORT OrAST : public ScopeAST
{
public:
    explicit OrAST( AST* parent );
    ~OrAST() override;
    QList<ScopeAST*> scopes;
};

class KDEVQMAKEPARSER_EXPORT ValueAST : public AST
{
public:
    explicit ValueAST( AST* parent );
    QString value;
};

}

#endif
