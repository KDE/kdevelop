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

#include "qmakeprojectfile.h"

#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QDir>

#include <kurl.h>
#include <kdebug.h>

#include "qmakeast.h"

const QStringList QMakeProjectFile::FileVariables = QStringList() << "IDLS"
        << "RESOURCES" << "IMAGES" << "LEXSOURCES" << "DISTFILES"
        << "YACCSOURCES" << "TRANSLATIONS" << "HEADERS" << "SOURCES"
        << "INTERFACES" << "FORMS" ;

QMakeProjectFile::QMakeProjectFile( const QString& projectfile )
    : QMakeFile( projectfile ), m_mkSpecs(0)
{
}

void QMakeProjectFile::setMkSpecs( QMakeMkSpecs* mkspecs )
{
    m_mkSpecs = mkspecs;
}

QList<QMakeProjectFile*> QMakeProjectFile::subProjects() const
{
    kDebug(9024) << k_funcinfo << "Fetching subprojects";
    QList<QMakeProjectFile*> list;
    foreach( QString subdir, variableValues( "SUBDIRS" ) )
    {
        QString fileOrPath;
        kDebug(9024) << k_funcinfo << "Found value:" << subdir;
        if ( containsVariable( subdir+".file" ) && !variableValues( subdir+".file" ).isEmpty() )
        {
            subdir = variableValues( subdir+".file" ).first();
        }else if( containsVariable( subdir+".subdir" ) && !variableValues( subdir+".subdir" ).isEmpty() )
        {
            subdir = variableValues( subdir+".subdir" ).first();
        }
        if( subdir.endsWith( ".pro" ) )
        {
            fileOrPath = resolveFileName( subdir.trimmed() );
        }else
        {
            fileOrPath = resolveFileName( subdir.trimmed() );
        }
        QMakeProjectFile* qmscope = new QMakeProjectFile( fileOrPath );
        qmscope->setMkSpecs( m_mkSpecs );
        if( qmscope->read() )
        {
            list.append( qmscope );
        }
    }

    kDebug(9024) << k_funcinfo << "found" << list.size() << "subprojects";
    return list;
}

KUrl::List QMakeProjectFile::files() const
{
    kDebug(9024) << k_funcinfo << "Fetching files";


    KUrl::List list;
    foreach( QString variable, QMakeProjectFile::FileVariables )
    {
        foreach( QString value, variableValues(variable) )
        {
            list << KUrl( resolveFileName( value ) );
        }
    }
    kDebug(9024) << k_funcinfo << "found" << list.size() << "files";
    return list;
}

KUrl::List QMakeProjectFile::filesForTarget( const QString& s ) const
{
    kDebug(9024) << k_funcinfo << "Fetching files";


    KUrl::List list;
    if( variableValues("INSTALLS").contains(s) )
    {
        QStringList files = variableValues(s+".files");
        if( !files.isEmpty() )
        {
            foreach( QString val, files )
            {
                list << KUrl( resolveFileName( val ) );
            }
        }
    }
    if( !variableValues("INSTALLS").contains(s) || s == "target" )
    {
        foreach( QString variable, QMakeProjectFile::FileVariables )
        {
            foreach( QString value, variableValues(variable) )
            {
                list << KUrl( resolveFileName( value ) );
            }
        }
    }
    kDebug(9024) << k_funcinfo << "found" << list.size() << "files";
    return list;
}

QString QMakeProjectFile::getTemplate() const
{
    QString templ = "app";
    if( !variableValues("TEMPLATE").isEmpty() )
    {
        templ = variableValues("TEMPLATE").first();
    }
    return templ;
}

QStringList QMakeProjectFile::targets() const
{
    kDebug(9024) << k_funcinfo << "Fetching targets";

    QStringList list;

    list += variableValues("TARGET");
    if( list.isEmpty() && getTemplate() != "subdirs" )
    {
        list += QFileInfo( absoluteFile() ).baseName();
    }

    foreach( QString target, variableValues("INSTALLS") )
    {
        if( target != "target" )
            list << target;
    }

    kDebug(9024) << k_funcinfo << "found" << list.size() << "targets";
    return list;
}

QMakeProjectFile::~QMakeProjectFile()
{
}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
