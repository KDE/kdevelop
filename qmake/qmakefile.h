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
#include <kurl.h>

namespace QMake
{
    class ProjectAST;
    class AssignmentAST;
    class ValueAST;
}

class Scope;

class QMakeFile : QMake::ASTDefaultVisitor
{
public:
    QMakeFile( const KUrl& file );
    virtual ~QMakeFile();
    bool read();
    KUrl absoluteDirUrl() const;
    KUrl absoluteFileUrl() const;
    QMake::ProjectAST* ast() const;

    void visitAssignment( QMake::AssignmentAST* node );
    void visitFunctionCall( QMake::FunctionCallAST* node );

    virtual QStringList variableValues(const QString&) const;
    bool containsVariable( const QString& ) const;

protected:
    QMap<QString, QStringList> m_variableValues;
private:
    QMake::ProjectAST* m_ast;
    KUrl m_projectFileUrl;
};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
