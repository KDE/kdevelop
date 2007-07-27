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
#include <QtCore/QStringList>

#include <kurl.h>
#include <kdebug.h>

#include "qmakeast.h"

const QStringList QMakeProjectScope::FileVariables = QStringList() << "IDLS"
        << "RESOURCES" << "IMAGES" << "LEXSOURCES" << "DISTFILES"
        << "YACCSOURCES" << "TRANSLATIONS" << "HEADERS" << "SOURCES"
        << "INTERFACES" << "FORMS" ;

QMakeProjectScope::QMakeProjectScope( const KUrl& projectfile )
    : QMakeFile(projectfile), m_mkSpecs(0)
{
}

void QMakeProjectScope::setMkSpecs( QMakeMkSpecs* mkspecs )
{
    m_mkSpecs = mkspecs;
}

QList<QMakeProjectScope*> QMakeProjectScope::subProjects() const
{
    kDebug(9024) << k_funcinfo << "Fetching subprojects" << endl;
    if( !ast() )
        return QList<QMakeProjectScope*>();

    kDebug(9024) << k_funcinfo << "I have " << ast()->statements().count() << " statements" << endl;

    QList<QMakeProjectScope*> list;
    foreach( QMake::StatementAST* stmt, ast()->statements() )
    {
        QMake::AssignmentAST* ast = dynamic_cast<QMake::AssignmentAST*>( stmt );

        if( ast && ast->variable()->value().contains("SUBDIRS") )
        {
            kDebug(9024) << k_funcinfo << "Found assignment: " << ast->variable()->value() << endl;
            foreach( QMake::ValueAST* value, ast->values() )
            {
                kDebug(9024) << k_funcinfo << "Found value: " << value->value() << endl;
                if( !value->value().trimmed().isEmpty() && value->value().trimmed() != "\\" )
                {
                    KUrl u = absoluteDirUrl();
                    u.adjustPath( KUrl::AddTrailingSlash );
                    u.setFileName( value->value().trimmed() );
                    QMakeProjectScope* qmscope = new QMakeProjectScope( u );
                    qmscope->setMkSpecs( m_mkSpecs );
                    list.append( qmscope );
                }
            }
        }
    }
    kDebug(9024) << k_funcinfo << "found " << list.size() << " subprojects" << endl;
    return list;
}

KUrl::List QMakeProjectScope::files() const
{
    kDebug(9024) << k_funcinfo << "Fetching files" << endl;
    if( !ast() )
        return KUrl::List();

    kDebug(9024) << k_funcinfo << "I have " << ast()->statements().count() << " statements" << endl;

    KUrl::List list;
    foreach( QMake::StatementAST* stmt, ast()->statements() )
    {
        QMake::AssignmentAST* ast = dynamic_cast<QMake::AssignmentAST*>( stmt );
        if( ast && QMakeProjectScope::FileVariables.indexOf( ast->variable()->value()  ) != -1 )
        {
            kDebug(9024) << k_funcinfo << "Found assignment: " << ast->variable()->value() << endl;
            foreach( QMake::ValueAST* value, ast->values() )
            {
                kDebug(9024) << k_funcinfo << "Found value: " << value->value() << endl;
                if( !value->value().trimmed().isEmpty() && value->value().trimmed() != "\\" )
                {
                    KUrl u = absoluteDirUrl();
                    u.adjustPath( KUrl::AddTrailingSlash );
                    u.setFileName( value->value().trimmed() );
                    list.append( u );
                }
            }
        }
    }
    list.append( absoluteFileUrl() );
    kDebug(9024) << k_funcinfo << "found " << list.size() << " files" << endl;
    return list;
}

QStringList QMakeProjectScope::targets() const
{
    kDebug(9024) << k_funcinfo << "Fetching targets" << endl;
    if( !ast() )
        return QStringList();

    kDebug(9024) << k_funcinfo << "I have " << ast()->statements().count() << " statements" << endl;

    QStringList list;
    foreach( QMake::StatementAST* stmt, ast()->statements() )
    {
        QMake::AssignmentAST* ast = dynamic_cast<QMake::AssignmentAST*>( stmt );
        if( ast && ast->variable()->value() == "INSTALLS" )
        {
            kDebug(9024) << k_funcinfo << "Found assignment: " << ast->variable()->value() << endl;
            foreach( QMake::ValueAST* value, ast->values() )
            {
                kDebug(9024) << k_funcinfo << "Found value: " << value->value() << endl;
                if( value->value().trimmed().isEmpty() && value->value().trimmed() != "\\" && value->value().trimmed() != "target" )
                {
                    list << value->value();
                }
            }
        }
    }
    kDebug(9024) << k_funcinfo << "found " << list.size() << " targets" << endl;
    return list;
}

QMakeProjectScope::~QMakeProjectScope()
{
}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
