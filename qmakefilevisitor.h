/*****************************************************************************
 * Copyright (c) 2010 Milian Wolff <mail@milianw.de>                         *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#ifndef QMAKEFILEVISITOR_H
#define QMAKEFILEVISITOR_H

#include "parser/qmakeastdefaultvisitor.h"

#include "variablereferenceparser.h"

#include <QHash>

class QMakeVariableResolver {
public:
    virtual ~QMakeVariableResolver() {}
    virtual QStringList resolveVariable(const QString& variable, VariableInfo::VariableType type) const = 0;

    typedef QHash< QString, QStringList > VariableMap;
};

class QMakeFile;

class QMakeFileVisitor : protected QMake::ASTDefaultVisitor, public QMakeVariableResolver {
public:
    explicit QMakeFileVisitor(const QMakeVariableResolver* resolver, QMakeFile* baseFile);
    virtual ~QMakeFileVisitor();

    virtual QStringList resolveVariable(const QString& variable, VariableInfo::VariableType type) const;

    // use this before visiting a file to set the default variables
    void setVariables(const VariableMap& vars);
    // visit whole file and return map of defined variables
    VariableMap visitFile(QMake::ProjectAST* node);
    // visit macro with given arguments and forward its returnvalue
    QStringList visitMacro(QMake::ScopeBodyAST* node, const QStringList& arguments);

protected:
    virtual void visitAssignment(QMake::AssignmentAST* node);
    virtual void visitFunctionCall(QMake::FunctionCallAST* node);

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
