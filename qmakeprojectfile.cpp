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

QHash<QString, QPair<QString, QString> > QMakeProjectFile::m_qmakeQueryCache = QHash<QString, QPair<QString, QString> >();

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
    // default values
    // NOTE: if we already have such a var, e.g. in an include file,
    //       we must not overwrite it here!
    if (!m_variableValues.contains("QT")) {
        m_variableValues["QT"] = QStringList() << "core" << "gui";
    }
    if (!m_variableValues.contains("CONFIG")) {
        m_variableValues["CONFIG"] = QStringList() << "qt";
    }

    Q_ASSERT(m_mkspecs);
    foreach( const QString& var, m_mkspecs->variables() )
    {
        if (!m_variableValues.contains(var)) {
            m_variableValues[var] = m_mkspecs->variableValues( var );
        }
    }
    if( m_cache )
    {
        foreach( const QString& var, m_cache->variables() )
        {
            if (!m_variableValues.contains(var)) {
                m_variableValues[var] = m_cache->variableValues( var );
            }
        }
    }

    ///TODO: more special variables
    m_variableValues["PWD"] = QStringList() << pwd();
    m_variableValues["_PRO_FILE_"] = QStringList() << proFile();
    m_variableValues["_PRO_FILE_PWD_"] = QStringList() << proFilePwd();
    m_variableValues["OUT_PWD"] = QStringList() << outPwd();

    const QString binary = QMakeConfig::qmakeBinary(project());
    if (!m_qmakeQueryCache.contains(binary)) {
        // Let's cache the Qt include dir
        KProcess qtInc;
        qtInc << binary << "-query" << "QT_INSTALL_HEADERS" << "QT_VERSION";
        qtInc.setOutputChannelMode( KProcess::OnlyStdoutChannel );
        qtInc.start();
        if ( !qtInc.waitForFinished() ) {
            kWarning() << "Failed to query Qt header path using qmake, is qmake installed?";
        } else {
            const QStringList result = QString::fromLocal8Bit(qtInc.readAll()).split(QRegExp("[:\n]"), QString::SkipEmptyParts);
            if (!result.size() == 4) {
                kWarning() << "Failed to query qmake - bad qmake binary configured?" << binary;
                m_qmakeQueryCache[binary] = qMakePair(QString(), QString());
            } else {
                m_qmakeQueryCache[binary] = qMakePair(result.at(1).trimmed(), result.at(3).trimmed());
            }
        }
    }
    m_qtIncludeDir = m_qmakeQueryCache.value(binary).first;
    m_qtVersion = m_qmakeQueryCache.value(binary).second;

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
            else if ( module == "widgets" )
                url.setPath(m_qtIncludeDir + "/QtWidgets");
            else if ( module == "webkitwidgets" )
                url.setPath(m_qtIncludeDir + "/QtWebKitWidgets");
            else if ( module == "quick" )
                url.setPath(m_qtIncludeDir + "/QtQuick");
            else if ( module == "concurrent" )
                url.setPath(m_qtIncludeDir + "/QtConcurrent");
            else if ( module == "location" )
                url.setPath(m_qtIncludeDir + "/QtLocation");
            else if ( module == "qml" )
                url.setPath(m_qtIncludeDir + "/QtQml");
            else if ( module == "bluetooth" )
                url.setPath(m_qtIncludeDir + "/QtBluetooth");
            else if ( module == "core-private" )
                url.setPath(m_qtIncludeDir + "/QtCore/" + m_qtVersion + "/QtCore/private/");
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
        if( !target.isEmpty() && target != "target" )
            list << target;
    }

    if (list.removeAll(QString())) {
        // remove empty targets - which is probably a bug...
        kWarning(9024) << "got empty entry in TARGET of file" << absoluteFile();
    }

    ifDebug(kDebug(9024) << "found" << list.size() << "targets";)
    return list;
}

QMakeProjectFile::~QMakeProjectFile()
{
    //TODO: delete cache, specs, ...?
}

QStringList QMakeProjectFile::resolveVariable(const QString& variable, VariableInfo::VariableType type) const
{
    if (type == VariableInfo::QtConfigVariable) {
        if (m_mkspecs->isQMakeInternalVariable(variable)) {
            return QStringList() << m_mkspecs->qmakeInternalVariable(variable);
        } else {
            kWarning(9024) << "unknown QtConfig Variable:" << variable;
            return QStringList();
        }
    }

    return QMakeFile::resolveVariable(variable, type);
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

QString QMakeProjectFile::pwd() const
{
    return absoluteDir();
}

QString QMakeProjectFile::outPwd() const
{
    if (!project()) {
        return absoluteDir();
    } else {
        return QMakeConfig::buildDirFromSrc(project(), absoluteDir()).toLocalFile();
    }
}

QString QMakeProjectFile::proFile() const
{
    return absoluteFile();
}

QString QMakeProjectFile::proFilePwd() const
{
    return absoluteDir();
}
