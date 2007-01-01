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

#include "qmakedriver.h"
#include "qmakeast.h"

const QStringList QMakeProjectScope::FileVariables = QStringList() << "IDLS"
        << "RESOURCES" << "IMAGES" << "LEXSOURCES" << "DISTFILES"
        << "YACCSOURCES" << "TRANSLATIONS" << "HEADERS" << "SOURCES"
        << "INTERFACES" << "FORMS" ;

QMakeProjectScope::QMakeProjectScope( const KUrl& projectfile )
    : m_ast(0)
{
    m_projectFileUrl = projectfile;
    QFileInfo fi( projectfile.toLocalFile() );
    m_ast = new QMake::ProjectAST();
    kDebug(9024) << k_funcinfo << "Is " << projectfile << " a dir?" << fi.isDir() << endl;
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
    if( QMake::Driver::parseFile( m_projectFileUrl.toLocalFile(), m_ast ) != 0 )
    {
        kDebug( 9024 ) << "Couldn't parse project: " << m_projectFileUrl.toLocalFile() << endl;
	delete m_ast;
        m_ast = 0;
        m_projectFileUrl = KUrl();
    }
}

QList<QMakeProjectScope*> QMakeProjectScope::subProjects() const
{
    kDebug(9024) << k_funcinfo << "Fetching subprojects" << endl;
    if( !m_ast )
        return QList<QMakeProjectScope*>();

    kDebug(9024) << k_funcinfo << "I have " << m_ast->statements().count() << " statements" << endl;

    QList<QMakeProjectScope*> list;
    foreach( QMake::StatementAST* stmt, m_ast->statements() )
    {
        QMake::AssignmentAST* ast = dynamic_cast<QMake::AssignmentAST*>( stmt );

        if( ast && ast->variable().contains("SUBDIRS") )
        {
            kDebug(9024) << k_funcinfo << "Found assignment: " << ast->variable() << endl;
            foreach( QString value, ast->values() )
            {
                kDebug(9024) << k_funcinfo << "Found value: " << value << endl;
                if( value.trimmed() != "" && value.trimmed() != "\\" )
                {
                    KUrl u = absoluteDirUrl();
                    u.adjustPath( KUrl::AddTrailingSlash );
                    u.setFileName( value.trimmed() );
                    list.append( new QMakeProjectScope( u ) );
                }
            }
        }
    }
    kDebug(9024) << k_funcinfo << "found " << list.size() << " subprojects" << endl;
    return list;
}

KUrl::List QMakeProjectScope::files() const
{
    kDebug(9024) << k_funcinfo << "Fetching subprojects" << endl;
    if( !m_ast )
        return KUrl::List();

    kDebug(9024) << k_funcinfo << "I have " << m_ast->statements().count() << " statements" << endl;

    KUrl::List list;
    foreach( QMake::StatementAST* stmt, m_ast->statements() )
    {
        QMake::AssignmentAST* ast = dynamic_cast<QMake::AssignmentAST*>( stmt );
        if( ast && QMakeProjectScope::FileVariables.indexOf( ast->variable()  ) != -1 )
        {
            kDebug(9024) << k_funcinfo << "Found assignment: " << ast->variable() << endl;
            foreach( QString value, ast->values() )
            {
                kDebug(9024) << k_funcinfo << "Found value: " << value << endl;
                if( value.trimmed() != "" && value.trimmed() != "\\" )
                {
                    KUrl u = absoluteDirUrl();
                    u.adjustPath( KUrl::AddTrailingSlash );
                    u.setFileName( value.trimmed() );
                    list.append( u );
                }
            }
        }
    }
    list.append( m_projectFileUrl );
    kDebug(9024) << k_funcinfo << "found " << list.size() << " subprojects" << endl;
    return list;
}

QStringList QMakeProjectScope::targets() const
{
    kDebug(9024) << k_funcinfo << "Fetching subprojects" << endl;
    if( !m_ast )
        return QStringList();

    kDebug(9024) << k_funcinfo << "I have " << m_ast->statements().count() << " statements" << endl;

    QStringList list;
    foreach( QMake::StatementAST* stmt, m_ast->statements() )
    {
        QMake::AssignmentAST* ast = dynamic_cast<QMake::AssignmentAST*>( stmt );
        if( ast && ast->variable() == "INSTALLS" )
        {
            kDebug(9024) << k_funcinfo << "Found assignment: " << ast->variable() << endl;
            foreach( QString value, ast->values() )
            {
                kDebug(9024) << k_funcinfo << "Found value: " << value << endl;
                if( value.trimmed() != "" && value.trimmed() != "\\" && value.trimmed() != "target" )
                {
                    list << value;
                }
            }
        }
    }
    kDebug(9024) << k_funcinfo << "found " << list.size() << " subprojects" << endl;
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
