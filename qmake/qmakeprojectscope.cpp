/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "qmakeprojectscope.h"

#include <QtCore/QList>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <kurl.h>
#include <kdebug.h>

#include "qmakeparser.h"
#include "qmakeast.h"

QMakeProjectScope::QMakeProjectScope( const KUrl& projectfile )
    : m_ast(0)
{
    m_projectFileUrl = projectfile;
    QFileInfo fi( projectfile.path() );
    if( fi.isDir() )
    {
        QDir dir( m_projectFileUrl.path() );
        QStringList l = dir.entryList( QStringList() << "*.pro" );

        QString projectfile;

        if( !l.count() || ( l.count() && l.indexOf( fi.baseName() + ".pro" ) != -1 ) )
        {
            projectfile = fi.baseName() + ".pro";
        }else
        {
            projectfile = l.first();
        }
        m_projectFileUrl.setFileName( projectfile );
    }
    if( !QMake::Parser::parseFile( m_projectFileUrl.pathOrUrl(), &m_ast ) )
    {
        kDebug( 9024 ) << "Couldn't parse project: " << m_projectFileUrl.pathOrUrl() << endl;
        m_ast = 0;
        m_projectFileUrl = KUrl();
    }
}

QList<QMakeProjectScope*> QMakeProjectScope::subProjects() const
{
    if( !m_ast )
        return QList<QMakeProjectScope*>();

    QList<QMakeProjectScope*> list;
    foreach( QMake::StatementAST* stmt, m_ast->statements() )
    {
        QMake::AssignmentAST* ast = dynamic_cast<QMake::AssignmentAST*>( stmt );
        if( ast && ast->variable().contains("SUBDIRS") )
        {
            foreach( QString value, ast->values() )
            {
                if( value.trimmed() != "" && value.trimmed() != "\\" )
                {
                    KUrl u = absoluteDirUrl();
                    u.setFileName( value.trimmed() );
                    list.append( new QMakeProjectScope( u ) );
                }
            }
        }
    }
    return list;
}

KUrl QMakeProjectScope::absoluteDirUrl() const
{
    return m_projectFileUrl.directory();
}

QMakeProjectScope::~QMakeProjectScope()
{
    delete m_ast;
    m_ast = 0;
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;
