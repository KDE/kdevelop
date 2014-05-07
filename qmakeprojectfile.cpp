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
#include <kdebug.h>

#include "parser/ast.h"
#include "qmakecache.h"
#include "qmakemkspecs.h"
#include "qmakeconfig.h"

#include <interfaces/iproject.h>
#include <util/path.h>

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
            if (result.size() != 4) {
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

void QMakeProjectFile::addPathsForVariable(const QString& variable, QStringList* list) const
{
    const QStringList values = variableValues(variable);
    ifDebug(kDebug(9024) << variable << values;)
    foreach( const QString& val, values ) {
        QString path = resolveToSingleFileName(val);
        if( !path.isEmpty() && !list->contains(val) ) {
            list->append(path);
        }
    }
}

QStringList QMakeProjectFile::includeDirectories() const
{
    ifDebug(kDebug(9024) << "Fetching include dirs" << m_qtIncludeDir;)
    ifDebug(kDebug(9024) << "CONFIG" << variableValues("CONFIG");)

    QStringList list;
    addPathsForVariable("INCLUDEPATH", &list);
    addPathsForVariable("QMAKE_INCDIR", &list);
    if( variableValues("CONFIG").contains("opengl") )
    {
        addPathsForVariable("QMAKE_INCDIR_OPENGL", &list);
    }
    if( variableValues("CONFIG").contains("qt") )
    {
        if( !list.contains( m_qtIncludeDir ) )
            list << m_qtIncludeDir;

        QDir incDir(m_qtIncludeDir);
        foreach( const QString& module, variableValues("QT") )
        {
            QString pattern = module;
            const bool isPrivate = module.endsWith("-private");
            if (isPrivate) {
                pattern.chop(strlen("-private"));
            }
            if (pattern == "qtestlib" || pattern == "testlib") {
                pattern = "QtTest";
            } else if ( pattern == "qaxcontainer" ) {
                pattern = "ActiveQt";
            } else if ( pattern == "qaxserver" ) {
                pattern = "ActiveQt";
            } else if ( pattern != "phonon" && pattern != "qt3support" ) {
                pattern.prepend("Qt");
            }
            const QFileInfoList match = incDir.entryInfoList(QStringList(pattern), QDir::Dirs);
            if (match.isEmpty()) {
                qWarning() << "unhandled Qt module:" << module << pattern;
                continue;
            }
            QString path = match.first().canonicalFilePath();
            if (isPrivate) {
                path += '/' + m_qtVersion + '/' + match.first().fileName() + "/private/";
            }
            if ( !list.contains(path) ) {
                list << path;
            }
        }
    }

    if( variableValues("CONFIG").contains("thread") )
    {
        addPathsForVariable("QMAKE_INCDIR_THREAD", &list);
    }
    if( variableValues("CONFIG").contains("x11") )
    {
        addPathsForVariable("QMAKE_INCDIR_X11", &list);
    }

    addPathsForVariable("MOC_DIR", &list);
    addPathsForVariable("OBJECTS_DIR", &list);
    addPathsForVariable("UI_DIR", &list);

    ifDebug(kDebug(9024) << "final list:" << list;)
    return list;
}

QStringList QMakeProjectFile::files() const
{
    ifDebug(kDebug(9024) << "Fetching files";)

    QStringList list;
    foreach( const QString& variable, QMakeProjectFile::FileVariables )
    {
        foreach( const QString& value, variableValues(variable) )
        {
            list += resolveFileName( value );
        }
    }
    ifDebug(kDebug(9024) << "found" << list.size() << "files";)
    return list;
}

QStringList QMakeProjectFile::filesForTarget( const QString& s ) const
{
    ifDebug(kDebug(9024) << "Fetching files";)

    QStringList list;
    if( variableValues("INSTALLS").contains(s) )
    {
        const QStringList files = variableValues(s+".files");
        if( !files.isEmpty() )
        {
            foreach( const QString& val, files )
            {
                list += QStringList( resolveFileName( val ) );
            }
        }
    }
    if( !variableValues("INSTALLS").contains(s) || s == "target" )
    {
        foreach( const QString& variable, QMakeProjectFile::FileVariables )
        {
            foreach( const QString& value, variableValues(variable) )
            {
                list += QStringList( resolveFileName( value ) );
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
        return QMakeConfig::buildDirFromSrc(project(), KDevelop::Path(absoluteDir())).toLocalFile();
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
