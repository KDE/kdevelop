/* KDevelop CMake Support
 *
 * Copyright 2008 Matt Rogers <mattr@kde.org>
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

#include "cmakeparserutils.h"

#include <QStringList>
#include <qfileinfo.h>
#include <qdir.h>
#include <kdebug.h>
#include "variablemap.h"
#include <kprocess.h>
#include <kstandarddirs.h>
#include "cmakeprojectvisitor.h"
#include "cmakeprojectdata.h"
#include "cmakecachereader.h"
#include <ktempdir.h>

namespace CMakeParserUtils
{
    QList<int> parseVersion(const QString& version, bool* ok)
    {
        QList<int> versionNumList;
        *ok = false;
        QStringList versionStringList = version.split('.', QString::SkipEmptyParts);

        foreach(const QString& part, versionStringList)
        {
            int i = part.toInt(ok);
            if (!*ok)
            {
                versionNumList.clear();
                return versionNumList;
            }
            else
                versionNumList.append(i);
        }
        return versionNumList;
    }

    QString valueFromSystemInfo(const QString& variable, const QString& systeminfo)
    {
        int idx = systeminfo.indexOf( variable );
        if( idx != -1 ) {
            // 2 == ' "'
            idx += variable.count() + 2;
            int end = systeminfo.indexOf( "\"", idx );
            if( end != -1 ) {
                return systeminfo.mid( idx, end-idx );
            }
        }
        return QString();
    }


    QPair<VariableMap,QStringList> initialVariables()
    {
        static QPair<VariableMap, QStringList> ret;
        if (!ret.first.isEmpty()) {
            return ret;
        }
        QString cmakeCmd=KStandardDirs::findExe("cmake");
        
        QString systeminfo=executeProcess(cmakeCmd, QStringList("--system-information"));
        
        VariableMap varsDef;
        QStringList modulePathDef=QStringList(CMakeParserUtils::valueFromSystemInfo( "CMAKE_ROOT", systeminfo ) + "/Modules");
        kDebug(9042) << "found module path is" << modulePathDef;
        varsDef.insertGlobal("CMAKE_BINARY_DIR", QStringList("#[bin_dir]"));
        varsDef.insertGlobal("CMAKE_INSTALL_PREFIX", QStringList("#[install_dir]"));
        varsDef.insertGlobal("CMAKE_COMMAND", QStringList(cmakeCmd));
        varsDef.insertGlobal("CMAKE_MAJOR_VERSION", QStringList(CMakeParserUtils::valueFromSystemInfo("CMAKE_MAJOR_VERSION", systeminfo)));
        varsDef.insertGlobal("CMAKE_MINOR_VERSION", QStringList(CMakeParserUtils::valueFromSystemInfo("CMAKE_MINOR_VERSION", systeminfo)));
        varsDef.insertGlobal("CMAKE_PATCH_VERSION", QStringList(CMakeParserUtils::valueFromSystemInfo("CMAKE_PATCH_VERSION", systeminfo)));
        varsDef.insertGlobal("CMAKE_INCLUDE_CURRENT_DIR", QStringList("OFF"));
        
        QStringList cmakeInitScripts;
        #ifdef Q_OS_WIN
            cmakeInitScripts << "CMakeMinGWFindMake.cmake";
            cmakeInitScripts << "CMakeMSYSFindMake.cmake";
            cmakeInitScripts << "CMakeNMakeFindMake.cmake";
            cmakeInitScripts << "CMakeVS8FindMake.cmake";
        #else
            cmakeInitScripts << "CMakeUnixFindMake.cmake";
        #endif
        cmakeInitScripts << "CMakeDetermineSystem.cmake";
        cmakeInitScripts << "CMakeSystemSpecificInformation.cmake";
        cmakeInitScripts << "CMakeDetermineCCompiler.cmake";
        cmakeInitScripts << "CMakeDetermineCXXCompiler.cmake";
        
        varsDef.insertGlobal("CMAKE_MODULE_PATH", modulePathDef);
        varsDef.insertGlobal("CMAKE_ROOT", QStringList(CMakeParserUtils::valueFromSystemInfo("CMAKE_ROOT", systeminfo)));
        
        //Defines the behaviour that can't be identified on initialization scripts
        #ifdef Q_OS_WIN32
            varsDef.insertGlobal("WIN32", QStringList("1"));
            varsDef.insertGlobal("CMAKE_HOST_WIN32", QStringList("1"));
        #else
            varsDef.insertGlobal("UNIX", QStringList("1"));
            varsDef.insertGlobal("CMAKE_HOST_UNIX", QStringList("1"));
        #endif
        #ifdef Q_OS_MAC
            varsDef.insertGlobal("APPLE", QStringList("1"));
            varsDef.insertGlobal("CMAKE_HOST_APPLE", QStringList("1"));
        #endif

        ret = qMakePair(varsDef, cmakeInitScripts);
        return ret;
    }

    QString executeProcess(const QString& execName, const QStringList& args)
    {
        Q_ASSERT(!execName.isEmpty());
        kDebug(9042) << "Executing:" << execName << "::" << args /*<< "into" << *m_vars*/;
        
        KProcess p;
        p.setOutputChannelMode(KProcess::MergedChannels);
        p.setProgram(execName, args);
        KTempDir tmp(KStandardDirs::locateLocal("tmp", "kdevcmakemanager"));
        p.setWorkingDirectory( tmp.name() );
        p.start();
        
        if(!p.waitForFinished())
        {
            kDebug() << "failed to execute:" << execName;
        }
        
        QByteArray b = p.readAllStandardOutput();
        QString t;
        t.prepend(b.trimmed());
        kDebug(9042) << "executed" << execName << "<" << t;
        tmp.unlink();
        return t;
    }

    void printSubdirectories(const QList<Subdirectory>& subs)
    {
        Q_FOREACH(const Subdirectory& s, subs) {
            qDebug() << "lala " << s.name;
        }
    }
    
    KDevelop::ReferencedTopDUContext includeScript(const QString& file, KDevelop::ReferencedTopDUContext parent, CMakeProjectData* data, const QString& sourcedir, const QMap< QString, QString >& env)
    {
        kDebug(9042) << "Running cmake script: " << file;
        CMakeFileContent f = CMakeListsParser::readCMakeFile(file);
        data->vm.insertGlobal("CMAKE_CURRENT_LIST_FILE", QStringList(file));
        data->vm.insertGlobal("CMAKE_CURRENT_LIST_DIR", QStringList(QFileInfo(file).dir().absolutePath()));

        const QString projectSourceDir = data->vm.value("CMAKE_SOURCE_DIR").first();
        const QString projectBinDir = data->vm.value("CMAKE_BINARY_DIR").join(QString());
        QString binDir = projectBinDir;
        // CURRENT_BINARY_DIR must point to the subfolder if any"
        if (sourcedir.startsWith(projectSourceDir)) {
            Q_ASSERT(projectSourceDir.size()==sourcedir.size() || sourcedir.at(projectSourceDir.size()) == '/');
            binDir += sourcedir.mid(projectSourceDir.size());
        }
        data->vm.insertGlobal("CMAKE_BINARY_DIR", QStringList(projectBinDir));
        data->vm.insertGlobal("CMAKE_CURRENT_BINARY_DIR", QStringList(binDir));
        data->vm.insertGlobal("CMAKE_CURRENT_SOURCE_DIR", QStringList(sourcedir));
        
        CMakeProjectVisitor v(file, parent);
        v.setCacheValues(&data->cache);
        v.setVariableMap(&data->vm);
        v.setMacroMap(&data->mm);
        v.setModulePath(data->modulePath);
        v.setEnvironmentProfile(env);
        v.setProperties(data->properties);
        v.walk(f, 0, true);
        
        data->projectName=v.projectName();
        data->subdirectories=v.subdirectories();
        data->targets=v.targets();
        data->properties=v.properties();
        data->testSuites=v.testSuites();
        
        //printSubdirectories(data->subdirectories);
        
        data->vm.remove("CMAKE_CURRENT_LIST_FILE");
        data->vm.remove("CMAKE_CURRENT_LIST_DIR");
        data->vm.remove("CMAKE_CURRENT_SOURCE_DIR");
        data->vm.remove("CMAKE_CURRENT_BINARY_DIR");
        
        return v.context();
    }
    
    CacheValues readCache(const KUrl &path)
    {
        QFile file(path.toLocalFile());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            kDebug() << "error. Could not find the file" << path;
            return CacheValues();
        }

        CacheValues ret;
        QTextStream in(&file);
        kDebug(9042) << "Reading cache:" << path;
        QStringList currentComment;
        while (!in.atEnd())
        {
            QString line = in.readLine().trimmed();
            if(!line.isEmpty() && line[0].isLetter()) //it is a variable
            {
                CacheLine c;
                c.readLine(line);
                if(c.flag().isEmpty()) {
                    ret[c.name()]=CacheEntry(c.value(), currentComment.join("\n"));
                    currentComment.clear();
                }
    //             kDebug(9042) << "Cache line" << line << c.name();
            }
            else if(line.startsWith("//"))
                currentComment += line.right(line.count()-2);
        }
        return ret;
    }
}
