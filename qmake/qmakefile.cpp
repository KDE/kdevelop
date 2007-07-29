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

#include "qmakefile.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <kdebug.h>

#include "qmakeast.h"
#include "qmakedriver.h"
#include "scope.h"

QMakeFile::QMakeFile( const KUrl& file )
    : m_ast(0)
{
    m_projectFileUrl = file;
    QFileInfo fi( file.toLocalFile() );
    kDebug(9024) << k_funcinfo << "Is " << file << " a dir?" << fi.isDir() << endl;
    if( fi.isDir() )
    {
        QDir dir( m_projectFileUrl.toLocalFile() );
        QStringList l = dir.entryList( QStringList() << "*.pro" );

        QString projectfile;

        if( !l.count() || ( l.count() && l.indexOf( fi.baseName() + ".pro" ) != -1 ) )
        {
            projectfile = fi.baseName() + ".pro";
        }else
        {
            projectfile = l.first();
        }
        m_projectFileUrl.adjustPath( KUrl::AddTrailingSlash );
        m_projectFileUrl.setFileName( projectfile );
    }
    QMake::Driver d;
    d.readFile( m_projectFileUrl.toLocalFile() );
    if( !d.parse( &m_ast ) )
    {
        kDebug( 9024 ) << "Couldn't parse project: " << m_projectFileUrl.toLocalFile() << endl;
        delete m_ast;
        m_ast = 0;
        m_projectFileUrl = KUrl();
    }else
    {
        kDebug(9024) << "found ast:" << m_ast->statements().count() << endl;
        visitNode(m_ast);
    }
}

QMakeFile::~QMakeFile()
{
    delete m_ast;
    m_ast = 0;
    delete m_topScope;
    m_topScope = 0;
    m_scopes.clear();
}

KUrl QMakeFile::absoluteDirUrl() const
{
    return m_projectFileUrl.directory();
}

KUrl QMakeFile::absoluteFileUrl() const
{
    return m_projectFileUrl;
}

QMake::ProjectAST* QMakeFile::ast() const
{
    return m_ast;
}

void QMakeFile::visitProject( QMake::ProjectAST* node )
{
    m_topScope = new Scope();
    m_topScope->setParent( m_scopes.top() );
    m_topScope->setAst( node );
    pushScope( m_topScope );
    QMake::ASTDefaultVisitor::visitProject( node );
    popScope();
}

void QMakeFile::visitScopeBody( QMake::ScopeBodyAST* node )
{
    Scope* s = new Scope();
    s->setParent( topScope() );
    s->setAst( node );
    topScope()->addSubScope(s);
    pushScope( s );
    QMake::ASTDefaultVisitor::visitScopeBody( node );
    popScope();
}

void QMakeFile::visitAssignment( QMake::AssignmentAST* node )
{
    topScope()->addVariable( node->variable()->value(), node );
}

Scope* QMakeFile::topScope() const
{
    if( m_scopes.isEmpty() )
        return 0;
    return m_scopes.top();
}

Scope* QMakeFile::topScope()
{
    if( m_scopes.isEmpty() )
        return 0;
    return m_scopes.top();
}


Scope* QMakeFile::popScope()
{
    if( m_scopes.isEmpty() )
        return 0;
    return m_scopes.pop();
}

void QMakeFile::pushScope( Scope* s )
{
    m_scopes.push(s);
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
