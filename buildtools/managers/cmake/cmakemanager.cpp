/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#include <QtDesigner/QExtensionFactory>

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
    kDebug(9032) << "Executing:" << execName << "::" << args /*<< "into" << *m_vars*/;

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
    kDebug(9032) << "executed" << execName << "<" << t;

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

    QString cmakeCmd = CMakeProjectVisitor::findFile("cmake", CMakeProjectVisitor::envVarDirectories("PATH"), CMakeProjectVisitor::Executable);
    m_modulePathDef=guessCMakeModulesDirectories(cmakeCmd);
    m_varsDef.insert("CMAKE_BINARY_DIR", QStringList("#[bin_dir]/"));
    m_varsDef.insert("CMAKE_INSTALL_PREFIX", QStringList("#[install_dir]/"));
    m_varsDef.insert("CMAKE_COMMAND", QStringList(cmakeCmd));
    m_varsDef.insert("CMAKE_MODULE_PATH", m_modulePathDef);

#if defined(Q_WS_X11) || defined(Q_WS_MAC) //If it has uname :)
    QString sysName=executeProcess("uname", QStringList("-s"));
    QString sysVersion=executeProcess("uname", QStringList("-r"));
    QString sysProcessor=executeProcess("uname", QStringList("-p"));
    
    m_varsDef.insert("UNIX", QStringList("TRUE"));
    m_varsDef.insert("CMAKE_SYSTEM_NAME", QStringList(sysName));
    m_varsDef.insert("CMAKE_SYSTEM_VERSION", QStringList(sysVersion));
    m_varsDef.insert("CMAKE_SYSTEM", QStringList(sysName+'-'+sysVersion));
    m_varsDef.insert("CMAKE_SYSTEM_PROCESSOR", QStringList(sysProcessor));
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
    kDebug(9032) << "modPath" << m_varsDef.value("CMAKE_MODULE_PATH");
}

CMakeProjectManager::~CMakeProjectManager()
{
    //delete m_rootItem;
}

KUrl CMakeProjectManager::buildDirectory(KDevelop::ProjectFolderItem *item) const
{
    KSharedConfig::Ptr cfg = item->project()->projectConfiguration();
    KConfigGroup group(cfg.data(), "CMake");
    KUrl path = group.readEntry("CurrentBuildDir");
//     KUrl projectPath = item->project()->folder();

    kDebug(9032) << "Build folder: " << path;
    return path;
}

KDevelop::ProjectFolderItem* CMakeProjectManager::import( KDevelop::IProject *project )
{
    CMakeFolderItem* m_rootItem;
    KUrl cmakeInfoFile(project->projectFileUrl());
    cmakeInfoFile = cmakeInfoFile.upUrl();
    QString folderUrl(cmakeInfoFile.toLocalFile());
    cmakeInfoFile.addPath("CMakeLists.txt");

    kDebug(9032) << "file is" << cmakeInfoFile.path();
    if ( !cmakeInfoFile.isLocalFile() )
    {
        //FIXME turn this into a real warning
        kWarning(9032) << "not a local file. CMake support doesn't handle remote projects" ;
    }
    else
    {
        VariableMap vm=m_varsDef;
        QStringList mpath=m_modulePathDef;
        
        KSharedConfig::Ptr cfg = project->projectConfiguration();
        KConfigGroup group(cfg.data(), "CMake");
        if(group.hasKey("CMakeDir"))
            mpath=group.readEntry("CMakeDir", QStringList());
        else
            group.writeEntry("CMakeDir", mpath);
        
        vm.insert("CMAKE_SOURCE_DIR", QStringList(cmakeInfoFile.upUrl().toLocalFile()));
        m_macrosPerProject[project]=MacroMap();
        m_modulePathPerProject[project]=mpath;
        m_varsPerProject[project]=vm;
        m_rootItem = new CMakeFolderItem(project, folderUrl, 0 );
        m_rootItem->setProjectRoot(true);
    }
    return m_rootItem;
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

    CMakeFileContent f = CMakeListsParser::readCMakeFile(cmakeListsPath.toLocalFile());

    VariableMap *vm=&m_varsPerProject[item->project()];
    MacroMap *mm=&m_macrosPerProject[item->project()];
    if(f.isEmpty())
    {
        kDebug(9032) << "There is no" << cmakeListsPath;
        return folderList;
    }

    new KDevelop::ProjectFileItem( item->project(), cmakeListsPath, folder );
    kDebug(9032) << "Adding cmake: " << cmakeListsPath << " to the model";

    QString currentBinDir=KUrl::relativeUrl(folder->project()->projectFileUrl().upUrl(), folder->url());
    vm->insert("CMAKE_CURRENT_BINARY_DIR", QStringList(vm->value("CMAKE_BINARY_DIR")[0]+currentBinDir));
    vm->insert("CMAKE_CURRENT_LIST_FILE", QStringList(cmakeListsPath.toLocalFile()));
    vm->insert("CMAKE_CURRENT_SOURCE_DIR", QStringList(folder->url().toLocalFile()));

    kDebug(9032) << "currentBinDir" << vm->value("CMAKE_CURRENT_BINARY_DIR");

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
        if(!v.targetDependencies(t).isEmpty()) //Just to remove verbosity
        {
            CMakeTargetItem* targetItem = new CMakeTargetItem( item->project(), t, folder );

            foreach( QString sFile, v.targetDependencies(t) )
            {
                if(sFile.isEmpty())
                    continue;
                KUrl sourceFile = folder->url();
                sourceFile.adjustPath( KUrl::AddTrailingSlash );
                sourceFile.addPath( sFile );
                new KDevelop::ProjectFileItem( item->project(), sourceFile, targetItem );
                kDebug(9032) << "..........Adding:" << sFile;
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

KUrl::List CMakeProjectManager::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>( item );
    while(!folder && folder->parent()!=0)
    {
        folder = dynamic_cast<CMakeFolderItem*>( item );
    }

    if(!folder)
        return KUrl::List();

    kDebug(9032) << "Include directories!" << folder->includeDirectories();
    return folder->includeDirectories();
}

KDevelop::IProjectBuilder * CMakeProjectManager::builder(KDevelop::ProjectFolderItem *) const
{
    return m_builder;
}

QStringList CMakeProjectManager::guessCMakeModulesDirectories(const QString& cmakeBin) const
{
    KUrl bin(cmakeBin);
    bin=bin.upUrl();
    bin=bin.upUrl();
    bin.cd("share/cmake-2.4/Modules");
//     QStringList env = CMakeProjectVisitor::envVarDirectories("CMAKEDIR");
// 
//     QStringList::iterator it=env.begin();
//     for(; it!=env.end(); ++it)
//         *it += "/Modules";
    kDebug(9032) << "guessing: " << bin.toLocalFile();
    return QStringList(bin.toLocalFile());
}

#include "cmakemanager.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;
