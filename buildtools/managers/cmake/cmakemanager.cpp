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
#include <QQueue>

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

#ifdef CMAKEDEBUGVISITOR
#include "cmakedebugvisitor.h"
#endif

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
    
    QStringList envVars;
#ifdef Q_OS_WIN
    envVars=CMakeProjectVisitor::envVarDirectories("Path");
#else
    envVars=CMakeProjectVisitor::envVarDirectories("PATH");
#endif
    QString cmakeCmd = CMakeProjectVisitor::findFile("cmake", envVars, CMakeProjectVisitor::Executable);
    m_modulePathDef=guessCMakeModulesDirectories(cmakeCmd);
    m_varsDef.insert("CMAKE_BINARY_DIR", QStringList("#[bin_dir]/"));
    m_varsDef.insert("CMAKE_INSTALL_PREFIX", QStringList("#[install_dir]/"));
    m_varsDef.insert("CMAKE_COMMAND", QStringList(cmakeCmd));
#ifdef Q_OS_WIN
    cmakeInitScripts << "CMakeMinGWFindMake.cmake";
    cmakeInitScripts << "CMakeMSYSFindMake.cmake";
    cmakeInitScripts << "CMakeNMakeFindMake.cmake";
    cmakeInitScripts << "CMakeVS8FindMake.cmake";
#else
    cmakeInitScripts << "CMakeUnixFindMake.cmake";
#endif
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

#ifdef Q_OS_WIN
    m_varsDef.insert("WIN32", QStringList("TRUE"));
#endif
    kDebug(9042) << "modPath" << m_varsDef.value("CMAKE_MODULE_PATH") << m_modulePathDef;
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

    kDebug(9032) << "Build folder: " << path;
    return path;
}

void CMakeProjectManager::initializeProject(KDevelop::IProject* project, const KUrl& baseUrl)
{
    m_macrosPerProject[project].clear();
    m_varsPerProject[project]=m_varsDef;
    m_varsPerProject[project].insert("CMAKE_SOURCE_DIR", QStringList(baseUrl.toLocalFile()));
        
    KSharedConfig::Ptr cfg = project->projectConfiguration();
    KConfigGroup group(cfg.data(), "CMake");
    if(group.hasKey("CMakeDir"))
    {
        QStringList l;
        foreach(QString path, group.readEntry("CMakeDir", QStringList()) )
        {
            if( QFileInfo(path).exists() )
            {
                m_modulePathPerProject[project] << path;
                l << path;
            }
        }
        if( !l.isEmpty() )
            group.writeEntry("CMakeDir", l);
        else
            group.writeEntry("CMakeDir", m_modulePathDef);
    }
    else
        group.writeEntry("CMakeDir", m_modulePathDef);
        
    foreach(const QString& script, cmakeInitScripts)
    {
        includeScript(CMakeProjectVisitor::findFile(script, m_modulePathPerProject[project]), project);
    }
}

KDevelop::ProjectFolderItem* CMakeProjectManager::import( KDevelop::IProject *project )
{
    CMakeFolderItem* m_rootItem;
    KUrl cmakeInfoFile(project->projectFileUrl());
    cmakeInfoFile = cmakeInfoFile.upUrl();
    QString folderUrl(cmakeInfoFile.toLocalFile());
    cmakeInfoFile.addPath("CMakeLists.txt");
    
    kDebug(9042) << "found module path is" << m_modulePathDef;

    kDebug(9042) << "file is" << cmakeInfoFile.path();
    if ( !cmakeInfoFile.isLocalFile() )
    {
        //FIXME turn this into a real warning
        kWarning(9032) << "error. not a local file. CMake support doesn't handle remote projects" ;
    }
    else
    {
        m_watchers[project] = new KDirWatch(project);
        m_modulePathPerProject[project]=m_modulePathDef;
        initializeProject(project, cmakeInfoFile.upUrl());
        
        m_rootItem = new CMakeFolderItem(project, folderUrl, 0 );
        m_rootItem->setProjectRoot(true);
        
        m_folderPerUrl[folderUrl]=m_rootItem;
        connect(m_watchers[project], SIGNAL(dirty(const QString&)), this, SLOT(dirtyFile(const QString&)));
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

    m_watchers[project]->addFile(file);
}

QList<KDevelop::ProjectFolderItem*> CMakeProjectManager::parse( KDevelop::ProjectFolderItem* item )
{
    QList<KDevelop::ProjectFolderItem*> folderList;
    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>( item );

    QStringList entries = QDir( item->url().toLocalFile() ).entryList( QDir::AllEntries | QDir::Hidden | QDir::System );
    
    entries.removeAll(".");
    entries.removeAll("..");
    
    if ( folder )
    {
        m_folderPerUrl[item->url()]=folder;
        
        kDebug(9042) << "parse:" << folder->url();
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

        kDebug(9042) << "Adding cmake: " << cmakeListsPath << " to the model";
        
        m_watchers[item->project()]->addFile(cmakeListsPath.toLocalFile());
        QString currentBinDir=KUrl::relativeUrl(folder->project()->projectFileUrl().upUrl(), folder->url());
        vm->insert("CMAKE_CURRENT_BINARY_DIR", QStringList(vm->value("CMAKE_BINARY_DIR")[0]+currentBinDir));
        vm->insert("CMAKE_CURRENT_LIST_FILE", QStringList(cmakeListsPath.toLocalFile()));
        vm->insert("CMAKE_CURRENT_SOURCE_DIR", QStringList(folder->url().toLocalFile()));
    
        kDebug(9042) << "currentBinDir" << vm->value("CMAKE_CURRENT_BINARY_DIR");
    
    #ifdef CMAKEDEBUGVISITOR
        CMakeAstDebugVisitor dv;
        dv.walk(cmakeListsPath.toLocalFile(), f, 0);
    #endif
        
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
            if( entries.contains( subf ) )
            {
                entries.removeAll( subf );
            }


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
                    
                    if( entries.contains( sourceFile.fileName() ) )
                    {
                        entries.removeAll( sourceFile.fileName() );
                    }
                    
                    new KDevelop::ProjectFileItem( item->project(), sourceFile, targetItem );
                    kDebug(9042) << "..........Adding:" << sourceFile;
                }
                m_targets.append(targetItem);
            }
        }
    }
    foreach( QString entry, entries )
    {
        KUrl folderurl = item->url();
        folderurl.addPath( entry );
        
        if( item->project()->inProject( folderurl ) )
            continue;
        if( QFileInfo( folderurl.toLocalFile() ).isDir() )
        {
            new KDevelop::ProjectFolderItem( item->project(), folderurl, item );
        }
        else
        {
            new KDevelop::ProjectFileItem( item->project(), folderurl, item );
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
//         kDebug(9042) << "replace? " << s;
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

    KUrl::List l = resolveSystemDirs(folder->project(), folder->includeDirectories());
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
    if(!d.exists(ret))
    {
        KUrl std(bin);
        std = std.upUrl();
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

void CMakeProjectManager::parseOnly(KDevelop::IProject* project, const KUrl &url)
{
    kDebug(9042) << "Looking for" << url << " to regenerate";

    KUrl cmakeListsPath(url);
    cmakeListsPath.addPath("CMakeLists.txt");

    VariableMap *vm=&m_varsPerProject[project];
    MacroMap *mm=&m_macrosPerProject[project];
    
    CMakeFileContent f = CMakeListsParser::readCMakeFile(cmakeListsPath.toLocalFile());
    if(f.isEmpty())
    {
        kDebug(9032) << "There is no" << cmakeListsPath;
        return;
    }
    
    QString currentBinDir=KUrl::relativeUrl(project->projectFileUrl().upUrl(), url);
    vm->insert("CMAKE_CURRENT_BINARY_DIR", QStringList(vm->value("CMAKE_BINARY_DIR")[0]+currentBinDir));
    vm->insert("CMAKE_CURRENT_LIST_FILE", QStringList(cmakeListsPath.toLocalFile()));
    vm->insert("CMAKE_CURRENT_SOURCE_DIR", QStringList(url.toLocalFile()));
    CMakeProjectVisitor v(url.toLocalFile());
    v.setVariableMap(vm);
    v.setMacroMap(mm);
    v.setModulePath(m_modulePathPerProject[project]);
    v.walk(f, 0);
    vm->remove("CMAKE_CURRENT_LIST_FILE");
    vm->remove("CMAKE_CURRENT_SOURCE_DIR");
    vm->remove("CMAKE_CURRENT_BINARY_DIR");
}

//Copied from ImportJob
void CMakeProjectManager::reimport(CMakeFolderItem* fi)
{
    QQueue< QList<KDevelop::ProjectFolderItem*> > workQueue;
    QList<KDevelop::ProjectFolderItem*> initial;
    initial.append( fi );
    workQueue.enqueue( initial );

    while( workQueue.count() > 0 )
    {
        QList<KDevelop::ProjectFolderItem*> front = workQueue.dequeue();
        Q_FOREACH( KDevelop::ProjectFolderItem* _item, front )
        {
            QList<KDevelop::ProjectFolderItem*> workingList = parse( _item );
            if( workingList.count() > 0 )
                workQueue.enqueue( workingList );
        }
    }
}

void CMakeProjectManager::dirtyFile(const QString & dirty)
{
    KUrl dir(dirty);
    kDebug(9042) << dir << " is dirty";
    if(dir.fileName() == "CMakeLists.txt")
    {
        dir=dir.upUrl();
        dir.adjustPath(KUrl::RemoveTrailingSlash);
        CMakeFolderItem *it=m_folderPerUrl[dir];
        QStandardItem *parent=it->parent();
        KDevelop::IProject* proj=it->project();
        
        KUrl projectBaseUrl=it->project()->projectFileUrl().upUrl();
        KUrl relative=KUrl::relativeUrl(projectBaseUrl, dir);
        
        it->model()->removeRow(it->index().row(), it->index().parent()); 
        /*initializeProject(proj, dir);
        KUrl current=projectBaseUrl;
        QStringList subs=relative.toLocalFile().split("/");
        subs.append(QString());
        for(; !subs.isEmpty(); current.cd(subs.takeFirst()))
        {
            parseOnly(proj, current);
        }*/
        
        CMakeFolderItem* fi=new CMakeFolderItem( proj, dir.toLocalFile(), parent);
        reimport(fi);
        m_folderPerUrl[dir]=fi;
    }
    else
    {
        kDebug(9042) << "Outside project file modified, should regenerate the whole project";
        //TODO: Should regenerate the whole project :D
    }
}

#include "cmakemanager.moc"
