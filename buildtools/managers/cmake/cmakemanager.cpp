/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
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

#include "cmakemanager.h"

#include <QList>
#include <QVector>
#include <QDomDocument>
#include <QDir>
// #include <QtDesigner/QExtensionFactory>

#include <KUrl>
#include <KProcess>
#include <kio/job.h>

#include <icore.h>
#include <iproject.h>
#include <iplugincontroller.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <projectmodel.h>

#include "cmakeconfig.h"
#include "cmakemodelitems.h"

#include "cmakeastvisitor.h"
#include "cmakeprojectvisitor.h"
#include "cmakeexport.h"
#include "icmakebuilder.h"

K_PLUGIN_FACTORY(CMakeSupportFactory, registerPlugin<CMakeProjectManager>(); )
K_EXPORT_PLUGIN(CMakeSupportFactory("kdevcmakemanager"))

QString executeProcess(const QString& execName, const QStringList& args=QStringList())
{
    kDebug(9042) << "Executing:" << execName << "::" << args /*<< "into" << *m_vars*/;

    KProcess p;
    p.setOutputChannelMode(KProcess::MergedChannels);
    p.setProgram(execName, args);
    p.start();

    if(!p.waitForFinished())
    {
        kDebug(9032) << "failed to execute:" << execName;
    }

    QByteArray b = p.readAllStandardOutput();
    QString t;
    t.prepend(b.trimmed());
    kDebug(9042) << "executed" << execName << "<" << t;

    return t;
}

CMakeProjectManager::CMakeProjectManager( QObject* parent, const QVariantList& )
    : KDevelop::IPlugin( CMakeSupportFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    IPlugin* i = core()->pluginController()->pluginForExtension( "org.kdevelop.ICMakeBuilder" );
    Q_ASSERT(i);
    if( i )
    {
        m_builder = i->extension<ICMakeBuilder>();
    }
    
    QString cmakeCmd;
#ifdef Q_WS_WIN
    cmakeCmd = CMakeProjectVisitor::findFile("cmake.exe", CMakeProjectVisitor::envVarDirectories("Path"), CMakeProjectVisitor::Executable);
#else
    cmakeCmd = CMakeProjectVisitor::findFile("cmake", CMakeProjectVisitor::envVarDirectories("PATH"), CMakeProjectVisitor::Executable);
#endif
    m_modulePathDef=guessCMakeModulesDirectories(cmakeCmd);
    m_varsDef.insert("CMAKE_BINARY_DIR", QStringList("#[bin_dir]/"));
    m_varsDef.insert("CMAKE_INSTALL_PREFIX", QStringList("#[install_dir]/"));
    m_varsDef.insert("CMAKE_COMMAND", QStringList(cmakeCmd));
    
    cmakeInitScripts << "CMakeUnixFindMake.cmake";
    cmakeInitScripts << "CMakeDetermineSystem.cmake";
    cmakeInitScripts << "CMakeDetermineCCompiler.cmake";
    cmakeInitScripts << "CMakeDetermineCXXCompiler.cmake";
    cmakeInitScripts << "CMakeSystemSpecificInformation.cmake";

    m_varsDef.insert("CMAKE_MODULE_PATH", m_modulePathDef);
    m_varsDef.insert("CMAKE_ROOT", QStringList(guessCMakeRoot(cmakeCmd)));

#if defined(Q_WS_X11) || defined(Q_WS_MAC)
    m_varsDef.insert("CMAKE_HOST_UNIX", QStringList("TRUE"));
    m_varsDef.insert("UNIX", QStringList("TRUE"));
    #ifdef Q_WS_X11
        m_varsDef.insert("LINUX", QStringList("TRUE"));
    #endif
    #ifdef Q_WS_MAC //NOTE: maybe should use __APPLE__
        m_varsDef.insert("APPLE", QStringList("TRUE"));
    #endif
#endif

#ifdef Q_WS_WIN
    m_varsDef.insert("WIN32", QStringList("TRUE"));
#endif
    kDebug(9042) << "modPath" << m_varsDef.value("CMAKE_MODULE_PATH");
}

CMakeProjectManager::~CMakeProjectManager()
{
    //delete m_rootItem;
}

KUrl CMakeProjectManager::buildDirectory(KDevelop::ProjectBaseItem *item) const
{
    KSharedConfig::Ptr cfg = item->project()->projectConfiguration();
    KConfigGroup group(cfg.data(), "CMake");
    KUrl path = group.readEntry("CurrentBuildDir");
//     KUrl projectPath = item->project()->folder();

    kDebug(9042) << "Build folder: " << path;
    return path;
}

KDevelop::ProjectFolderItem* CMakeProjectManager::import( KDevelop::IProject *project )
{
    CMakeFolderItem* m_rootItem;
    KUrl cmakeInfoFile(project->projectFileUrl());
    cmakeInfoFile = cmakeInfoFile.upUrl();
    QString folderUrl(cmakeInfoFile.toLocalFile());
    cmakeInfoFile.addPath("CMakeLists.txt");

    kDebug(9042) << "file is" << cmakeInfoFile.path();
    if ( !cmakeInfoFile.isLocalFile() )
    {
        //FIXME turn this into a real warning
        kWarning(9032) << "not a local file. CMake support doesn't handle remote projects" ;
    }
    else
    {
        m_macrosPerProject[project].clear();
        m_modulePathPerProject[project]=m_modulePathDef;
        m_varsPerProject[project]=m_varsDef;

        KSharedConfig::Ptr cfg = project->projectConfiguration();
        KConfigGroup group(cfg.data(), "CMake");
        if(group.hasKey("CMakeDir"))
            m_modulePathPerProject[project]=group.readEntry("CMakeDir", QStringList());
        else
            group.writeEntry("CMakeDir", m_modulePathDef);

        m_varsPerProject[project].insert("CMAKE_SOURCE_DIR", QStringList(cmakeInfoFile.upUrl().toLocalFile()));
        
        foreach(const QString& script, cmakeInitScripts) {
            includeScript(CMakeProjectVisitor::findFile(script, m_modulePathPerProject[project]), project);
        }
        
        m_rootItem = new CMakeFolderItem(project, folderUrl, 0 );
        m_rootItem->setProjectRoot(true);
    }
    return m_rootItem;
}


void CMakeProjectManager::includeScript(const QString& file, KDevelop::IProject * project)
{
    kDebug(9042) << "Running cmake script: " << file;
    VariableMap *vm=&m_varsPerProject[project];
    MacroMap *mm=&m_macrosPerProject[project];
    CMakeFileContent f = CMakeListsParser::readCMakeFile(file);
    if(f.isEmpty())
    {
        kDebug(9032) << "There is no such file: " << file;
        return;
    }

    vm->insert("CMAKE_CURRENT_BINARY_DIR", QStringList(vm->value("CMAKE_BINARY_DIR")[0]));
    vm->insert("CMAKE_CURRENT_LIST_FILE", QStringList(file));

    CMakeProjectVisitor v(file);
    v.setVariableMap(vm);
    v.setMacroMap(mm);
    v.setModulePath(m_modulePathPerProject[project]);
    v.walk(f, 0);

    vm->remove("CMAKE_CURRENT_LIST_FILE");
    vm->remove("CMAKE_CURRENT_SOURCE_DIR");
    vm->remove("CMAKE_CURRENT_BINARY_DIR");
}

QList<KDevelop::ProjectFolderItem*> CMakeProjectManager::parse( KDevelop::ProjectFolderItem* item )
{
    QList<KDevelop::ProjectFolderItem*> folderList;
    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>( item );

    if ( !folder )
        return folderList;

    kDebug(9032) << "parse:" << folder->url();
    KUrl cmakeListsPath(folder->url());
    cmakeListsPath.addPath("CMakeLists.txt");

    VariableMap *vm=&m_varsPerProject[item->project()];
    MacroMap *mm=&m_macrosPerProject[item->project()];
    CMakeFileContent f = CMakeListsParser::readCMakeFile(cmakeListsPath.toLocalFile());
    if(f.isEmpty())
    {
        kDebug(9032) << "There is no" << cmakeListsPath;
        return folderList;
    }

    new KDevelop::ProjectFileItem( item->project(), cmakeListsPath, folder );
    kDebug(9042) << "Adding cmake: " << cmakeListsPath << " to the model";

    QString currentBinDir=KUrl::relativeUrl(folder->project()->projectFileUrl().upUrl(), folder->url());
    vm->insert("CMAKE_CURRENT_BINARY_DIR", QStringList(vm->value("CMAKE_BINARY_DIR")[0]+currentBinDir));
    vm->insert("CMAKE_CURRENT_LIST_FILE", QStringList(cmakeListsPath.toLocalFile()));
    vm->insert("CMAKE_CURRENT_SOURCE_DIR", QStringList(folder->url().toLocalFile()));

    kDebug(9042) << "currentBinDir" << vm->value("CMAKE_CURRENT_BINARY_DIR");

    CMakeProjectVisitor v(folder->url().toLocalFile());
    v.setVariableMap(vm);
    v.setMacroMap(mm);
    v.setModulePath(m_modulePathPerProject[item->project()]);
    v.walk(f, 0);
    vm->remove("CMAKE_CURRENT_LIST_FILE");
    vm->remove("CMAKE_CURRENT_SOURCE_DIR");
    vm->remove("CMAKE_CURRENT_BINARY_DIR");

    if(folder->text()=="/" && !v.projectName().isEmpty())
    {
        folder->setText(v.projectName());
    }

    foreach ( QString subf, v.subdirectories() )
    {
        KUrl path(folder->url());
        path.addPath(subf);

        CMakeFolderItem* a = new CMakeFolderItem( item->project(), subf, folder );
        a->setUrl(path);
        folderList.append( a );
    }

    KUrl::List directories;
    foreach(QString s, v.includeDirectories())
    {
        KUrl path;
        if(s.startsWith('/'))
        {
            path=s;
        }
        else
        {
            path=folder->url();
            path.addPath(s);
        }
        directories.append(path);
    }
    folder->setIncludeDirectories(directories);

    foreach ( QString t, v.targets())
    {
        QStringList dependencies=v.targetDependencies(t);
        if(!dependencies.isEmpty()) //Just to remove verbosity
        {
            CMakeTargetItem* targetItem = new CMakeTargetItem( item->project(), t, folder );

            foreach( QString sFile, dependencies )
            {
                if(sFile.isEmpty())
                    continue;
                KUrl sourceFile(sFile);
                if(sourceFile.isRelative()) {
                    sourceFile = folder->url();
                    sourceFile.adjustPath( KUrl::AddTrailingSlash );
                    sourceFile.addPath( sFile );
                }
                new KDevelop::ProjectFileItem( item->project(), sourceFile, targetItem );
                kDebug(9042) << "..........Adding:" << sourceFile;
            }
            m_targets.append(targetItem);
        }
    }
    return folderList;
}

QList<KDevelop::ProjectTargetItem*> CMakeProjectManager::targets() const
{
    return m_targets;
}

KUrl::List resolveSystemDirs(KDevelop::IProject* project, const KUrl::List& dirs)
{
    KSharedConfig::Ptr cfg = project->projectConfiguration();
    KConfigGroup group(cfg.data(), "CMake");
    QString bindir=group.readEntry("CurrentBuildDir", QString());
    QString instdir=group.readEntry("CurrentBuildDir", QString());

    //FIXME: I wonder how i could do it better
    KUrl::List newList;
    foreach(KUrl u, dirs)
    {
        QString s=u.toLocalFile();
//         kDebug(9032) << "replace? " << s;
        if(s.startsWith(QString::fromUtf8("#[bin_dir]"))) {
            s=s.replace("#[bin_dir]", bindir);
            u=KUrl(s);
            kDebug(9042) << "bindir" << u;
        } else if(s.startsWith(QString::fromUtf8("#[install_dir]"))) {
            s=s.replace("#[install_dir]", bindir);
            u=KUrl(s);
            kDebug(9042) << "installdir" << u;
        }
        newList.append(u);
    }
    return newList;
}

KUrl::List CMakeProjectManager::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    CMakeFolderItem* folder=0;
    kDebug(9042) << "Querying inc dirs for " << item;
    while(!folder && item)
    {
        folder = dynamic_cast<CMakeFolderItem*>( item );
        item = static_cast<KDevelop::ProjectBaseItem*>(item->parent());
        kDebug(9042) << "Looking for a folder: " << folder << item;
    }

    if(!folder)
        return KUrl::List();

    KUrl::List l = resolveSystemDirs(item->project(), folder->includeDirectories());
    kDebug(9042) << "Include directories!" << l;
    return l;
}

KDevelop::IProjectBuilder * CMakeProjectManager::builder(KDevelop::ProjectFolderItem *) const
{
    return m_builder;
}

QString CMakeProjectManager::guessCMakeShare(const QString& cmakeBin)
{
    KUrl bin(cmakeBin);
    bin=bin.upUrl();
    bin=bin.upUrl();
    return bin.toLocalFile();
}

QString CMakeProjectManager::guessCMakeRoot(const QString & cmakeBin)
{
    QString ret;
    KUrl bin(guessCMakeShare(cmakeBin));

    QString version=executeProcess(cmakeBin, QStringList("--version"));
    QRegExp rx("[a-z* ]*([0-9.]*)-[0-9]*");
    rx.indexIn(version);
    QString versionNumber = rx.capturedTexts()[1];

    bin.cd(QString("share/cmake-%1/").arg(versionNumber));

    ret=bin.toLocalFile();
    QDir d(ret);
    if(!d.exists(ret)) {
        KUrl std(bin);
        std = std.upUrl(); std = std.upUrl();
        std.cd("cmake/");
        ret=std.toLocalFile();
        bin = std;
    }

    kDebug(9042) << "guessing: " << bin.toLocalFile();
    return ret;
}

QStringList CMakeProjectManager::guessCMakeModulesDirectories(const QString& cmakeBin)
{
    return QStringList(guessCMakeRoot(cmakeBin)+"/Modules");
}

#include "cmakemanager.moc"

