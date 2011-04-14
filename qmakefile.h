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

#include "qmakeastdefaultvisitor.h"

#include <QtCore/QStack>
#include <QtCore/QStringList>

#include <KUrl>

namespace QMake
{
    class ProjectAST;
    class AssignmentAST;
    class ValueAST;
}

class Scope;
class QMakeMkSpecs;

class QMakeFile : QMake::ASTDefaultVisitor
{
public:
    QMakeFile( const QString& file );
    virtual ~QMakeFile();
    virtual bool read();
    QString absoluteDir() const;
    QString absoluteFile() const;
    QMake::ProjectAST* ast() const;

    void visitAssignment( QMake::AssignmentAST* node );
    void visitFunctionCall( QMake::FunctionCallAST* node );

    QStringList variableValues(const QString&) const;
    QStringList variables() const;
    typedef QMap< QString, QStringList > VariableMap;
    VariableMap variableMap() const;

    bool containsVariable( const QString& ) const;

    static QStringList resolveShellGlobbing( const QString& absolutefile );
    virtual QStringList resolveVariables( const QString& value ) const;


protected:
    VariableMap m_variableValues;
    QStringList resolveFileName( const QString& file ) const;
    QString resolveToSingleFileName( const QString& file ) const;
private:

    QStringList getValueList( const QList<QMake::ValueAST*>& list ) const;

    QStringList evaluateMacro( const QString& function, const QStringList& arguments ) const;

    QMake::ProjectAST* m_ast;
    QString m_projectFile;
    QMap<QString, QMake::ScopeBodyAST*> m_userMacros;
    QStringList m_arguments;
    QStringList m_lastReturn;
};

#endif

//kate: hl c++;
