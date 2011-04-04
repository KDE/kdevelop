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

#include <kprocess.h>
#include <kurl.h>
#include <kdebug.h>

#include "parser/ast.h"
#include "qmakecache.h"
#include "qmakemkspecs.h"
#include "qmakeconfig.h"

#include <interfaces/iproject.h>

#define ifDebug(x)

QString QMakeProjectFile::m_qtIncludeDir = QString();

const QStringList QMakeProjectFile::FileVariables = QStringList() << "IDLS"
        << "RESOURCES" << "IMAGES" << "LEXSOURCES" << "DISTFILES"
        << "YACCSOURCES" << "TRANSLATIONS" << "HEADERS" << "SOURCES"
        << "INTERFACES" << "FORMS" ;

QMakeProjectFile::QMakeProjectFile( const QString& projectfile )
    : QMakeFile( projectfile ), m_mkspecs(0), m_cache(0), m_project(0)
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
    Q_ASSERT(m_mkspecs);
    foreach( const QString& var, m_mkspecs->variables() )
    {
        m_variableValues[var] = m_mkspecs->variableValues( var );
    }
    if( m_cache )
    {
        foreach( const QString& var, m_cache->variables() )
        {
            m_variableValues[var] = m_cache->variableValues( var );
        }
    }

    ///TODO: more special variables
    m_variableValues["PWD"] = QStringList() << absoluteDir();
    m_variableValues["_PRO_FILE_"] = QStringList() << absoluteFile();
    m_variableValues["_PRO_FILE_PWD_"] = m_variableValues["PWD"];
    m_variableValues["OUT_PWD"] = QStringList() << buildDir().toLocalFile();

    if (m_qtIncludeDir.isEmpty()) {
        // Let's cache the Qt include dir
        KProcess qtInc;
        qtInc << "qmake" << "-query" << "QT_INSTALL_HEADERS";
        qtInc.setOutputChannelMode( KProcess::OnlyStdoutChannel );
        qtInc.start();
        if ( !qtInc.waitForFinished() ) {
            kWarning() << "Failed to query Qt header path using qmake, is qmake installed?";
        } else {
            QByteArray result = qtInc.readAll();
            m_qtIncludeDir = QString::fromLocal8Bit( result ).trimmed();
        }
    }

    return QMakeFile::read();
}

QStringList QMakeProjectFile::subProjects() const
{
    ifDebug(kDebug(9024) << "Fetching subprojects";)
    QStringList list;
    foreach(  QString subdir, variableValues( "SUBDIRS" ) )
    {
        QString fileOrPath;
        ifDebug(kDebug(9024) << "Found value:" << subdir;)
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
        if (fileOrPath.isEmpty()) {
            kWarning() << "could not resolve subdir" << subdir << "to file or path, skipping";
            continue;
        }
        list << fileOrPath;
    }

    ifDebug(kDebug(9024) << "found" << list.size() << "subprojects";)
    return list;
}

bool QMakeProjectFile::hasSubProject(const QString& file) const
{
    foreach( const QString& sub, subProjects() ) {
        if (sub == file) {
            return true;
        } else if ( QFileInfo(file).absoluteDir() == sub ) {
            return true;
        }
    }
    return false;
}

void QMakeProjectFile::addUrlsForVariable(const QString& variable, KUrl::List* list) const
{
    const QStringList values = variableValues(variable);
    ifDebug(kDebug(9024) << variable << values;)
    foreach( const QString& val, values ) {
        KUrl url( resolveToSingleFileName(val) );
        if( url.isValid() && !list->contains(val) ) {
            list->append(url);
        }
    }
}

KUrl::List QMakeProjectFile::includeDirectories() const
{
    ifDebug(kDebug(9024) << "Fetching include dirs" << m_qtIncludeDir;)
    ifDebug(kDebug(9024) << "CONFIG" << variableValues("CONFIG");)

    KUrl::List list;
    addUrlsForVariable("INCLUDEPATH", &list);
    addUrlsForVariable("QMAKE_INCDIR", &list);
    if( variableValues("CONFIG").contains("opengl") )
    {
        addUrlsForVariable("QMAKE_INCDIR_OPENGL", &list);
    }
    if( variableValues("CONFIG").contains("qt") )
    {
        KUrl url(m_qtIncludeDir);
        if( !list.contains( url ) )
            list << url;

        foreach( const QString& module, variableValues("QT") )
        {
            KUrl url;
            if ( module == "core" )
                url.setPath(m_qtIncludeDir + "/QtCore");
            else if ( module == "gui" )
                url.setPath(m_qtIncludeDir + "/QtGui");
            else if ( module == "network" )
                url.setPath(m_qtIncludeDir + "/QtNetwork");
            else if ( module == "opengl" )
                url.setPath(m_qtIncludeDir + "/QtOpenGL");
            else if ( module == "phonon" )
                url.setPath(m_qtIncludeDir + "/Phonon");
            else if ( module == "script" )
                url.setPath(m_qtIncludeDir + "/QtScript");
            else if ( module == "scripttools" )
                url.setPath(m_qtIncludeDir + "/QtScriptTools");
            else if ( module == "sql" )
                url.setPath(m_qtIncludeDir + "/QtSql");
            else if ( module == "svg" )
                url.setPath(m_qtIncludeDir + "/QtSvg");
            else if ( module == "webkit" )
                url.setPath(m_qtIncludeDir + "/QtWebKit");
            else if ( module == "xml" )
                url.setPath(m_qtIncludeDir + "/QtXml");
            else if ( module == "xmlpatterns" )
                url.setPath(m_qtIncludeDir + "/QtXmlPatterns");
            else if ( module == "qt3support" )
                url.setPath(m_qtIncludeDir + "/Qt3Support");
            else if ( module == "designer" )
                url.setPath(m_qtIncludeDir + "/QtDesigner");
            else if ( module == "uitools" )
                url.setPath(m_qtIncludeDir + "/QtUiTools");
            else if ( module == "help" )
                url.setPath(m_qtIncludeDir + "/QtHelp");
            else if ( module == "assistant" )
                url.setPath(m_qtIncludeDir + "/QtAssistant");
            else if ( module == "qtestlib" || module == "testlib" )
                url.setPath(m_qtIncludeDir + "/QtTest");
            else if ( module == "qaxcontainer" )
                url.setPath(m_qtIncludeDir + "/ActiveQt");
            else if ( module == "qaxserver" )
                url.setPath(m_qtIncludeDir + "/ActiveQt");
            else if ( module == "dbus" )
                url.setPath(m_qtIncludeDir + "/QtDBus");
            else if ( module == "declarative" )
                url.setPath(m_qtIncludeDir + "/QtDeclarative");
            else {
                kWarning() << "unhandled QT module:" << module;
                continue;
            }

            if( !list.contains( url ) )
                list << url;
        }
    }

    if( variableValues("CONFIG").contains("thread") )
    {
        addUrlsForVariable("QMAKE_INCDIR_THREAD", &list);
    }
    if( variableValues("CONFIG").contains("x11") )
    {
        addUrlsForVariable("QMAKE_INCDIR_X11", &list);
    }

    addUrlsForVariable("MOC_DIR", &list);
    addUrlsForVariable("OBJECTS_DIR", &list);
    addUrlsForVariable("UI_DIR", &list);

    ifDebug(kDebug(9024) << "final list:" << list;)
    return list;
}

KUrl::List QMakeProjectFile::files() const
{
    ifDebug(kDebug(9024) << "Fetching files";)

    KUrl::List list;
    foreach( const QString& variable, QMakeProjectFile::FileVariables )
    {
        foreach( const QString& value, variableValues(variable) )
        {
            list += KUrl::List( resolveFileName( value ) );
        }
    }
    ifDebug(kDebug(9024) << "found" << list.size() << "files";)
    return list;
}

KUrl::List QMakeProjectFile::filesForTarget( const QString& s ) const
{
    ifDebug(kDebug(9024) << "Fetching files";)

    KUrl::List list;
    if( variableValues("INSTALLS").contains(s) )
    {
        const QStringList files = variableValues(s+".files");
        if( !files.isEmpty() )
        {
            foreach( const QString& val, files )
            {
                list += KUrl::List( resolveFileName( val ) );
            }
        }
    }
    if( !variableValues("INSTALLS").contains(s) || s == "target" )
    {
        foreach( const QString& variable, QMakeProjectFile::FileVariables )
        {
            foreach( const QString& value, variableValues(variable) )
            {
                list += KUrl::List( resolveFileName( value ) );
            }
        }
    }
    ifDebug(kDebug(9024) << "found" << list.size() << "files";)
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
    ifDebug(kDebug(9024) << "Fetching targets";)

    QStringList list;

    list += variableValues("TARGET");
    if( list.isEmpty() && getTemplate() != "subdirs" )
    {
        list += QFileInfo( absoluteFile() ).baseName();
    }

    foreach( const QString& target, variableValues("INSTALLS") )
    {
        if( target != "target" )
            list << target;
    }

    ifDebug(kDebug(9024) << "found" << list.size() << "targets";)
    return list;
}

QMakeProjectFile::~QMakeProjectFile()
{
    //TODO: delete cache, specs, ...?
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

QList< QMakeProjectFile::DefinePair > QMakeProjectFile::defines() const
{
    QList< DefinePair > d;
    foreach(QString def, variableMap()["DEFINES"])
    {
        int pos = def.indexOf('=');
        if(pos >= 0)
        {
            // a value is attached to define
            d.append(DefinePair( def.left(pos), def.right(def.length() - (pos+1)) ));
        }
        else
        {
            // a value-less define
            d.append(DefinePair( def, "" ));
        }
    }
    return d;
}

void QMakeProjectFile::setProject(KDevelop::IProject* project)
{
    m_project = project;
}

KDevelop::IProject* QMakeProjectFile::project() const
{
    return m_project;
}

KUrl QMakeProjectFile::buildDir() const
{
    const KUrl url(absoluteDir());
    if (!m_project) {
        return url;
    } else {
        return QMakeConfig::buildDirFromSrc(m_project, url);
    }
}

