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
#include "qmakecache.h"
#include "qmakemkspecs.h"

const QStringList QMakeProjectFile::FileVariables = QStringList() << "IDLS"
        << "RESOURCES" << "IMAGES" << "LEXSOURCES" << "DISTFILES"
        << "YACCSOURCES" << "TRANSLATIONS" << "HEADERS" << "SOURCES"
        << "INTERFACES" << "FORMS" ;

QMakeProjectFile::QMakeProjectFile( const QString& projectfile )
    : QMakeFile( projectfile ), m_mkspecs(0), m_cache(0)
{
}

void QMakeProjectFile::setQMakeCache( QMakeCache* cache )
{
    m_cache = cache;
}

void QMakeProjectFile::setMkSpecs( QMakeMkSpecs* mkspecs )
{
    m_mkspecs = mkspecs;
}

bool QMakeProjectFile::read()
{
    if( m_cache )
    {
        foreach( QString var, m_cache->variables() )
        {
            m_variableValues[var] = m_cache->variableValues( var );
        }
    }else
    {

        foreach( QString var, m_mkspecs->variables() )
        {
            m_variableValues[var] = m_mkspecs->variableValues( var );
        }
    }
    return QMakeFile::read();
}

QList<QMakeProjectFile*> QMakeProjectFile::subProjects() const
{
    kDebug(9024) << "Fetching subprojects";
    QList<QMakeProjectFile*> list;
    foreach( QString subdir, variableValues( "SUBDIRS" ) )
    {
        QString fileOrPath;
        kDebug(9024) << "Found value:" << subdir;
        if ( containsVariable( subdir+".file" ) && !variableValues( subdir+".file" ).isEmpty() )
        {
            subdir = variableValues( subdir+".file" ).first();
        }else if( containsVariable( subdir+".subdir" ) && !variableValues( subdir+".subdir" ).isEmpty() )
        {
            subdir = variableValues( subdir+".subdir" ).first();
        }
        if( subdir.endsWith( ".pro" ) )
        {
            fileOrPath = resolveToSingleFileName( subdir.trimmed() );
        }else
        {
            fileOrPath = resolveToSingleFileName( subdir.trimmed() );
        }
        Q_ASSERT( !fileOrPath.isEmpty() );
        QMakeProjectFile* qmscope = new QMakeProjectFile( fileOrPath );
        QDir d;
        if( QFileInfo( fileOrPath ).isDir() )
        {
            d = QDir( fileOrPath );
        }else
        {
            d = QFileInfo( fileOrPath ).dir();
        }
        if( d.exists(".qmake.cache") )
        {
            QMakeCache* cache = new QMakeCache( d.canonicalPath()+"/.qmake.cache" );
            cache->setMkSpecs( m_mkspecs );
            cache->read();
            qmscope->setQMakeCache( cache );
        }else
        {
            qmscope->setQMakeCache( m_cache );
        }
        qmscope->setMkSpecs( m_mkspecs );
        if( qmscope->read() )
        {
            list.append( qmscope );
        }
    }

    kDebug(9024) << "found" << list.size() << "subprojects";
    return list;
}

KUrl::List QMakeProjectFile::includeDirectories() const
{
    kDebug(9024) << "Fetching include dirs";

    KUrl::List list;
    kDebug(9024) << variableValues("INCLUDEPATH");
    foreach( QString val, variableValues("INCLUDEPATH") )
    {
        KUrl url(val);
        if( !list.contains( url ) )
            list << url;
    }
    kDebug(9024) << variableValues("QMAKE_INCDIR");
    foreach( QString val, variableValues("QMAKE_INCDIR") )
    {
        KUrl url(val);
        if( !list.contains( url ) )
            list << url;
    }
    kDebug(9024) << variableValues("QMAKE_INCDIR_OPENGL");
    if( variableValues("CONFIG").contains("opengl") )
    {
        foreach( QString val, variableValues("QMAKE_INCDIR_OPENGL") )
        {
            KUrl url(val);
            if( !list.contains( url ) )
                list << url;
        }
    }
    kDebug(9024) << variableValues("QMAKE_INCDIR_QT");
    if( variableValues("CONFIG").contains("qt") )
    {
        //@TODO add QtCore,QtGui and so on depending on CONFIG values,
        //      as QMAKE_INCDIR_QT only contains the include/ dir
        foreach( QString val, variableValues("QMAKE_INCDIR_QT") )
        {
            KUrl url(val);
            if( !list.contains( url ) )
                list << url;
        }
    }
    kDebug(9024) << variableValues("QMAKE_INCDIR_THREAD");
    if( variableValues("CONFIG").contains("thread") )
    {
        foreach( QString val, variableValues("QMAKE_INCDIR_THREAD") )
        {
            KUrl url(val);
            if( !list.contains( url ) )
                list << url;
        }
    }
    kDebug(9024) << variableValues("QMAKE_INCDIR_X11");
    if( variableValues("CONFIG").contains("x11") )
    {
        foreach( QString val, variableValues("QMAKE_INCDIR_X11") )
        {
            KUrl url(val);
            if( !list.contains( url ) )
                list << url;
        }
    }
    return list;
}

KUrl::List QMakeProjectFile::files() const
{
    kDebug(9024) << "Fetching files";


    KUrl::List list;
    foreach( QString variable, QMakeProjectFile::FileVariables )
    {
        foreach( QString value, variableValues(variable) )
        {
            list += KUrl::List( resolveFileName( value ) );
        }
    }
    kDebug(9024) << "found" << list.size() << "files";
    return list;
}

KUrl::List QMakeProjectFile::filesForTarget( const QString& s ) const
{
    kDebug(9024) << "Fetching files";


    KUrl::List list;
    if( variableValues("INSTALLS").contains(s) )
    {
        QStringList files = variableValues(s+".files");
        if( !files.isEmpty() )
        {
            foreach( QString val, files )
            {
                list += KUrl::List( resolveFileName( val ) );
            }
        }
    }
    if( !variableValues("INSTALLS").contains(s) || s == "target" )
    {
        foreach( QString variable, QMakeProjectFile::FileVariables )
        {
            foreach( QString value, variableValues(variable) )
            {
                list += KUrl::List( resolveFileName( value ) );
            }
        }
    }
    kDebug(9024) << "found" << list.size() << "files";
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
    kDebug(9024) << "Fetching targets";

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

    kDebug(9024) << "found" << list.size() << "targets";
    return list;
}

QMakeProjectFile::~QMakeProjectFile()
{
}

QStringList QMakeProjectFile::resolveVariables( const QString& value ) const
{
    QString mkspecresolved = m_mkspecs->resolveInternalQMakeVariables( value );
    return QMakeFile::resolveVariables( mkspecresolved );
}

QMakeMkSpecs* QMakeProjectFile::mkSpecs() const
{
    return m_mkspecs;
}

QMakeCache* QMakeProjectFile::qmakeCache() const
{
    return m_cache;
}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
