/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QMAKEFILE_H
#define QMAKEFILE_H


#include <util/stack.h>

#include "qmakefilevisitor.h"

#include <QStringList>

namespace KDevelop
{
class IProject;
}

namespace QMake
{
    class ProjectAST;
}

class Scope;
class QMakeMkSpecs;

class QMakeFile : public QMakeVariableResolver
{
public:
    explicit QMakeFile( QString  file );
    ~QMakeFile() override;
    virtual bool read();
    QString absoluteDir() const;
    QString absoluteFile() const;
    QMake::ProjectAST* ast() const;

    QStringList variableValues(const QString&) const;
    QStringList variables() const;
    VariableMap variableMap() const;

    bool containsVariable( const QString& ) const;

    QStringList resolveVariable( const QString& variable, VariableInfo::VariableType type) const override;

    /// required for proper build-dir resolution
    void setProject(KDevelop::IProject* project);
    KDevelop::IProject* project() const;
protected:
    VariableMap m_variableValues;

    QStringList resolveShellGlobbing( const QString& pattern, const QString& base = {} ) const;
    QStringList resolveFileName( const QString& file, const QString& base = {} ) const;
    QString resolveToSingleFileName( const QString& file, const QString& base = {} ) const;
private:
    QMake::ProjectAST* m_ast;
    QString m_projectFile;
    KDevelop::IProject* m_project;
};

#endif
