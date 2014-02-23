/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef QMAKEFILE_H
#define QMAKEFILE_H


#include <QtCore/QStack>
#include <QtCore/QStringList>

#include "qmakefilevisitor.h"

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
    QMakeFile( const QString& file );
    virtual ~QMakeFile();
    virtual bool read();
    QString absoluteDir() const;
    QString absoluteFile() const;
    QMake::ProjectAST* ast() const;

    QStringList variableValues(const QString&) const;
    QStringList variables() const;
    VariableMap variableMap() const;

    bool containsVariable( const QString& ) const;

    virtual QStringList resolveVariable( const QString& variable, VariableInfo::VariableType type) const;

    /// required for proper build-dir resolution
    void setProject(KDevelop::IProject* project);
    KDevelop::IProject* project() const;
protected:
    VariableMap m_variableValues;

    QStringList resolveShellGlobbing( const QString& pattern ) const;
    QStringList resolveFileName( const QString& file ) const;
    QString resolveToSingleFileName( const QString& file ) const;
private:
    QMake::ProjectAST* m_ast;
    QString m_projectFile;
    KDevelop::IProject* m_project;
};

#endif

//kate: hl c++;
