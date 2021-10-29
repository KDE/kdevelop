/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMAKEFILEVISITOR_H
#define QMAKEFILEVISITOR_H

#include "parser/qmakeastdefaultvisitor.h"

#include "variablereferenceparser.h"

#include <QHash>

class QMakeVariableResolver {
public:
    virtual ~QMakeVariableResolver() {}
    virtual QStringList resolveVariable(const QString& variable, VariableInfo::VariableType type) const = 0;

    using VariableMap = QHash<QString, QStringList>;
};

class QMakeFile;

class QMakeFileVisitor : protected QMake::ASTDefaultVisitor, public QMakeVariableResolver {
public:
    explicit QMakeFileVisitor(const QMakeVariableResolver* resolver, QMakeFile* baseFile);
    ~QMakeFileVisitor() override;

    QStringList resolveVariable(const QString& variable, VariableInfo::VariableType type) const override;

    // use this before visiting a file to set the default variables
    void setVariables(const VariableMap& vars);
    // visit whole file and return map of defined variables
    VariableMap visitFile(QMake::ProjectAST* node);
    // visit macro with given arguments and forward its returnvalue
    QStringList visitMacro(QMake::ScopeBodyAST* node, const QStringList& arguments);

protected:
    void visitAssignment(QMake::AssignmentAST* node) override;
    void visitFunctionCall(QMake::FunctionCallAST* node) override;

private:
    QStringList resolveVariables( const QString& value ) const;

    QStringList getValueList( const QList<QMake::ValueAST*>& list ) const;

    QStringList evaluateMacro( const QString& function, const QStringList& arguments ) const;

    const QMakeVariableResolver* const m_resolver;
    QMakeFile* m_baseFile;
    VariableMap m_variableValues;

    QHash<QString, QMake::ScopeBodyAST*> m_userMacros;
    QStringList m_arguments;
    QStringList m_lastReturn;
};

#endif // QMAKEFILEVISITOR_H
