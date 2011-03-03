/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007-2009 Aleix Pol <aleixpol@kde.org>
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

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KAboutData>
#include <KDialog>
#include <kparts/mainwindow.h>
#include <KUrl>
#include <KAction>
#include <KMessageBox>
#include <ktexteditor/document.h>
#include <KStandardDirs>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <project/projectmodel.h>
#include <project/importprojectjob.h>
#include <project/helper.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/duchain.h>
#include <language/duchain/dumpchain.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/codecompletion/codecompletion.h>

#include "cmakemodelitems.h"
#include "cmakenavigationwidget.h"
#include "cmakecachereader.h"
#include "cmakeastvisitor.h"
#include "cmakeprojectvisitor.h"
#include "cmakeexport.h"
#include "cmakecodecompletionmodel.h"
#include "cmakeutils.h"
#include "cmaketypes.h"
#include "parser/cmakeparserutils.h"
#include "icmakedocumentation.h"

#ifdef CMAKEDEBUGVISITOR
#include "cmakedebugvisitor.h"
#endif

#include "ui_cmakepossibleroots.h"
#include <language/duchain/use.h>
#include <interfaces/idocumentation.h>
#include "cmakeprojectdata.h"
#include <cmakeconfig.h>

#include <language/highlighting/codehighlighting.h>
#include <interfaces/iruncontroller.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/vcsjob.h>
#include <project/interfaces/iprojectbuilder.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(CMakeSupportFactory, registerPlugin<CMakeManager>(); )
K_EXPORT_PLUGIN(CMakeSupportFactory(KAboutData("kdevcmakemanager","kdevcmake", ki18n("CMake Manager"), "0.1", ki18n("Support for managing CMake projects"), KAboutData::License_GPL)))

Q_DECLARE_METATYPE ( KDevelop::ProjectFolderItem* )

namespace {

QString fetchBuildDir(KDevelop::IProject* project)
{
    Q_ASSERT(project);
    return CMake::currentBuildDir(project).toLocalFile(KUrl::AddTrailingSlash);
}

QString fetchInstallDir(KDevelop::IProject* project)
{
    Q_ASSERT(project);
    return CMake::currentInstallDir(project).toLocalFile(KUrl::AddTrailingSlash);
}

inline QString replaceBuildDir(QString in, QString buildDir)
{
    return in.replace("#[bin_dir]", buildDir);
}

inline  QString replaceInstallDir(QString in, QString installDir)
{
    return in.replace("#[install_dir]", installDir);
}

KUrl::List resolveSystemDirs(KDevelop::IProject* project, const QStringList& dirs)
{
    QString buildDir = fetchBuildDir(project);
    QString installDir = fetchInstallDir(project);

    KUrl::List newList;
    foreach(const QString& _s, dirs)
    {
        QString s=_s;
        if(s.startsWith(QString::fromUtf8("#[bin_dir]")))
        {
            s= replaceBuildDir(s, buildDir);
        }
        else if(s.startsWith(QString::fromUtf8("#[install_dir]")))
        {
            s= replaceInstallDir(s, installDir);
        }
//         kDebug(9042) << "resolving" << _s << "to" << s;
        newList.append(KUrl(s));
    }
    return newList;
}

}

CMakeManager::CMakeManager( QObject* parent, const QVariantList& )
    : KDevelop::IPlugin( CMakeSupportFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ILanguageSupport )
    KDEV_USE_EXTENSION_INTERFACE( ICMakeManager)

    m_highlight = new KDevelop::CodeHighlighting(this);

    new CodeCompletion(this, new CMakeCodeCompletionModel(this), name());
    
    connect(ICore::self()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)), SLOT(projectClosing(KDevelop::IProject*)));
}

CMakeManager::~CMakeManager()
{}

KUrl CMakeManager::buildDirectory(KDevelop::ProjectBaseItem *item) const
{
    KUrl ret;
    ProjectBaseItem* parent = item->parent();
    if (parent)
        ret=buildDirectory(parent);
    else
        ret=CMake::currentBuildDir(item->project());
    
    CMakeFolderItem *fi=dynamic_cast<CMakeFolderItem*>(item);
    if(fi)
        ret.addPath(fi->buildDir());
    return ret;
}

KDevelop::ReferencedTopDUContext CMakeManager::initializeProject(KDevelop::IProject* project)
{
    KUrl baseUrl=project->folder();
    baseUrl.cd(CMake::projectRootRelative(project));
    
    QPair<VariableMap,QStringList> initials = CMakeParserUtils::initialVariables();
    CMakeProjectData* data = &m_projectsData[project];
    
    data->clear();
    data->modulePath=initials.first["CMAKE_MODULE_PATH"];
    data->vm=initials.first;
    data->vm.insert("CMAKE_SOURCE_DIR", QStringList(baseUrl.toLocalFile(KUrl::RemoveTrailingSlash)));
    
    KUrl cachefile=buildDirectory(project->projectItem());
    cachefile.addPath("CMakeCache.txt");
    data->cache=readCache(cachefile);

    KSharedConfig::Ptr cfg = project->projectConfiguration();
    KConfigGroup group(cfg.data(), "CMake");
    if(group.hasKey("CMakeDir"))
    {
        QStringList l;
        foreach(const QString &path, group.readEntry("CMakeDir", QStringList()) )
        {
            if( QFileInfo(path).exists() )
            {
                data->modulePath << path;
                l << path;
            }
        }
        if( !l.isEmpty() )
            group.writeEntry("CMakeDir", l);
        else
            group.writeEntry("CMakeDir", data->modulePath);
    }
    else
        group.writeEntry("CMakeDir", data->modulePath);

    
    KDevelop::ReferencedTopDUContext buildstrapContext;
    {
        KUrl buildStrapUrl = baseUrl;
        buildStrapUrl.addPath("buildstrap");
        DUChainWriteLocker lock(DUChain::lock());
        
        buildstrapContext = DUChain::self()->chainForDocument(buildStrapUrl);
        
        if(buildstrapContext) {
            buildstrapContext->clearLocalDeclarations();
            buildstrapContext->clearImportedParentContexts();
            buildstrapContext->deleteChildContextsRecursively();
        }else{
            IndexedString idxpath(buildStrapUrl);
            buildstrapContext=new TopDUContext(idxpath, RangeInRevision(0,0, 0,0),
                                               new ParsingEnvironmentFile(idxpath));
            DUChain::self()->addDocumentChain(buildstrapContext);
        }
        
        Q_ASSERT(buildstrapContext);
    }
    ReferencedTopDUContext ref=buildstrapContext;
    foreach(const QString& script, initials.second)
    {
        ref = includeScript(CMakeProjectVisitor::findFile(script, m_projectsData[project].modulePath, QStringList()), project, baseUrl.toLocalFile(), ref);
    }
    
    //Initialize parent parts of the project that don't belong to the tree (because it's a partial import)
    if(baseUrl.isParentOf(project->folder()) && baseUrl!=project->folder())
    {
        QList<KUrl> toimport;
        toimport += baseUrl;
        while(!toimport.isEmpty()) {
            KUrl script = toimport.takeFirst(), currentDir=script;
            script.addPath("CMakeLists.txt");
            
            ref = includeScript(script.toLocalFile(), project, currentDir.toLocalFile(), ref);
            Q_ASSERT(ref);
            
            foreach(const Subdirectory& s, data->subdirectories) {
                KUrl candidate = currentDir;
                candidate.addPath(s.name);
                
                if(candidate.isParentOf(project->folder()))
                    toimport += candidate;
            }
        }
        
        dynamic_cast<CMakeFolderItem*>(project->projectItem())->setBuildDir(KUrl::relativeUrl(baseUrl, project->folder()));
    }
    return ref;
}

KDevelop::ProjectFolderItem* CMakeManager::import( KDevelop::IProject *project )
{
    CMakeFolderItem* m_rootItem=0;
    KUrl cmakeInfoFile(project->projectFileUrl());
    cmakeInfoFile = cmakeInfoFile.upUrl();
    cmakeInfoFile.addPath("CMakeLists.txt");

    KUrl folderUrl=project->folder();
    kDebug(9042) << "file is" << cmakeInfoFile.toLocalFile();

    if ( !cmakeInfoFile.isLocalFile() )
    {
        kWarning() << "error. not a local file. CMake support doesn't handle remote projects";
    }
    else
    {
        KSharedConfig::Ptr cfg = project->projectConfiguration();
        KConfigGroup group(cfg.data(), "CMake");

        if(group.hasKey("ProjectRootRelative"))
        {
            QString relative=CMake::projectRootRelative(project);
            folderUrl.cd(relative);
        }
        else
        {
            KDialog chooseRoot;
            QWidget *e=new QWidget(&chooseRoot);
            Ui::CMakePossibleRoots ui;
            ui.setupUi(e);
            chooseRoot.setMainWidget(e);
            for(KUrl aux=folderUrl; QFile::exists(aux.toLocalFile()+"/CMakeLists.txt"); aux=aux.upUrl())
                ui.candidates->addItem(aux.toLocalFile());

            if(ui.candidates->count()>1)
            {
                connect(ui.candidates, SIGNAL(itemActivated(QListWidgetItem*)), &chooseRoot,SLOT(accept()));
                ui.candidates->setMinimumSize(384,192);
                int a=chooseRoot.exec();
                if(!a || !ui.candidates->currentItem())
                {
                    return 0;
                }
                KUrl choice=KUrl(ui.candidates->currentItem()->text());
                CMake::setProjectRootRelative(project, KUrl::relativeUrl(folderUrl, choice));
                folderUrl=choice;
            }
            else
            {
                CMake::setProjectRootRelative(project, "./");
            }
        }

        m_rootItem = new CMakeFolderItem(project, project->folder(), QString(), 0 );

        KUrl cachefile=buildDirectory(m_rootItem);
        if( cachefile.isEmpty() ) {
            CMake::checkForNeedingConfigure(m_rootItem);
        }
        cachefile.addPath("CMakeCache.txt");
        
        KDirWatch* w = new KDirWatch(project);
        w->setObjectName(project->name()+"_ProjectWatcher");
        w->addFile(cachefile.toLocalFile());
        connect(w, SIGNAL(dirty(QString)), this, SLOT(dirtyFile(QString)));
        connect(w, SIGNAL(deleted(QString)), this, SLOT(deletedWatched(QString)));
        m_watchers[project] = w;
        Q_ASSERT(m_rootItem->rowCount()==0);
    }
    return m_rootItem;
}


KDevelop::ReferencedTopDUContext CMakeManager::includeScript(const QString& file,
                                                        KDevelop::IProject * project, const QString& dir, ReferencedTopDUContext parent)
{
    m_watchers[project]->addFile(file);
    return CMakeParserUtils::includeScript( file, parent, &m_projectsData[project], dir);
}



QSet<QString> filterFiles(const QStringList& orig)
{
    QSet<QString> ret;
    foreach(const QString& str, orig)
    {
        ///@todo This filter should be configurable, and filtering should be done on a manager-independent level
        if (str.endsWith(QLatin1Char('~')) || str.endsWith(QLatin1String(".bak")))
            continue;

        ret.insert(str);
    }
    return ret;
}

QList<KDevelop::ProjectFolderItem*> CMakeManager::parse( KDevelop::ProjectFolderItem* item )
{
    Q_ASSERT(isReloading(item->project()));
    QList<KDevelop::ProjectFolderItem*> folderList;
    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>( item );

    m_watchers[item->project()]->addDir(item->url().toLocalFile(), KDirWatch::WatchFiles);
    
    KUrl cmakeListsPath(folder->url());
    cmakeListsPath.addPath("CMakeLists.txt");
    
    if(folder && QFile::exists(cmakeListsPath.toLocalFile()))
    {
        kDebug(9042) << "parse:" << folder->url();
        
        KDevelop::ReferencedTopDUContext curr;
        if(item==item->project()->projectItem())
            curr=initializeProject(item->project());
        else
            curr=folder->formerParent()->topDUContext();
        
        kDebug(9042) << "Adding cmake: " << cmakeListsPath << " to the model";

        QString binDir=KUrl::relativePath(folder->project()->projectItem()->url().toLocalFile(), folder->url().toLocalFile());
        if(binDir.startsWith("./"))
            binDir=binDir.remove(0, 2);
        
        CMakeProjectData& data=m_projectsData[item->project()];

//         kDebug(9042) << "currentBinDir" << KUrl(data.vm.value("CMAKE_BINARY_DIR")[0]) << data.vm.value("CMAKE_CURRENT_BINARY_DIR");

    #ifdef CMAKEDEBUGVISITOR
        CMakeAstDebugVisitor dv;
        dv.walk(cmakeListsPath.toLocalFile(), f, 0);
    #endif

        ReferencedTopDUContext ctx = includeScript(cmakeListsPath.toLocalFile(), folder->project(), item->url().toLocalFile(), curr);
        folder->setTopDUContext(ctx);
       /*{
        kDebug() << "dumpiiiiiing" << folder->url();
        KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
        KDevelop::dumpDUContext(v.context(), false);
        }*/

        QStringList alreadyAdded;
        folder->cleanupBuildFolders(data.subdirectories);
        foreach (const Subdirectory& subf, data.subdirectories)
        {
            if(subf.name.isEmpty() || alreadyAdded.contains(subf.name)) //empty case would not be necessary if we didn't process the wrong lines
                continue;
            
            KUrl path(subf.name);
            if(path.isRelative())
            {
                path=folder->url();
                path.addPath(subf.name);
            }
            path.adjustPath(KUrl::AddTrailingSlash);
            
            if(QDir(path.toLocalFile()).exists())
            {
                alreadyAdded.append(subf.name);
                CMakeFolderItem* parent=folder;
                if(path.upUrl()!=folder->url())
                    parent=0;

                CMakeFolderItem* a = 0;
                if(ProjectFolderItem* ff = folder->folderNamed(subf.name))
                {
                    if(ff->type()!=ProjectBaseItem::BuildFolder)
                        delete ff;
                    else
                        a = static_cast<CMakeFolderItem*>(ff);
                    
                }
                if(!a)
                    a = new CMakeFolderItem( folder->project(), path, subf.build_dir, parent );
                else
                    a->setUrl(path);
                
//                 kDebug() << "folder: " << a << a->index();
                a->setDefinitions(data.definitions);
                folderList.append( a );
                
                if(!parent) {
                    m_pending[path]=a;
                    a->setFormerParent(folder);
                }

                DescriptorAttatched* datt=static_cast<DescriptorAttatched*>(a);
                datt->setDescriptor(subf.desc);
            }
        }

//         if(folderList.isEmpty() && path.isParentOf(item->url()))
//             kDebug() << "poor guess";

        QStringList directories;
        directories += folder->url().toLocalFile(KUrl::RemoveTrailingSlash);

        foreach(const QString& s, data.includeDirectories)
        {
            QString dir(s);
            if(!s.startsWith("#["))
            {
                if(KUrl( s ).isRelative())
                {
                    KUrl path=folder->url();
                    path.addPath(s);
                    dir=path.toLocalFile();
                }

                KUrl simp(dir); //We use this to simplify dir
                simp.cleanPath();
                dir=simp.toLocalFile();
            }

            if(!directories.contains(dir))
                directories.append(dir);
        }
        folder->setIncludeDirectories(directories);
//             kDebug(9042) << "setting include directories: " << folder->url() << directories << "result: " << folder->includeDirectories();
        folder->setDefinitions(data.definitions);

        folder->cleanupTargets(data.targets);
        foreach ( const Target& t, data.targets)
        {
            QStringList files=t.files;
            QString outputName=t.name;
            if(data.properties[TargetProperty].contains(t.name) && data.properties[TargetProperty][t.name].contains("OUTPUT_NAME"))
                outputName=data.properties[TargetProperty][t.name]["OUTPUT_NAME"].first();
            
            QString path;
            switch(t.type)
            {
                case Target::Library:
                    path=data.vm.value("CMAKE_LIBRARY_OUTPUT_DIRECTORY").join(QString());
                    break;
                case Target::Executable:
                    path=data.vm.value("CMAKE_RUNTIME_OUTPUT_DIRECTORY").join(QString());
                    break;
                case Target::Custom:
                    break;
            }
            
            KUrl resolvedPath;
            if(!path.isEmpty())
                resolvedPath=resolveSystemDirs(folder->project(), QStringList(path)).first();
            
            KDevelop::ProjectTargetItem* targetItem = folder->targetNamed(t.type, t.name);
            if (!targetItem)
                switch(t.type)
                {
                    case Target::Library:
                        targetItem = new CMakeLibraryTargetItem( item->project(), t.name,
                                                                folder, t.declaration, outputName, resolvedPath);
                        break;
                    case Target::Executable:
                        targetItem = new CMakeExecutableTargetItem( item->project(), t.name,
                                                                    folder, t.declaration, outputName, resolvedPath);
                        break;
                    case Target::Custom:
                        targetItem = new CMakeCustomTargetItem( item->project(), t.name,
                                                                folder, t.declaration, outputName );
                        break;
                }
            DescriptorAttatched* datt=dynamic_cast<DescriptorAttatched*>(targetItem);
            datt->setDescriptor(t.desc);

            KUrl::List tfiles;
            foreach( const QString & sFile, t.files)
            {
                if(sFile.startsWith("#[") || sFile.isEmpty())
                    continue;

                KUrl sourceFile(sFile);
                if(sourceFile.isRelative()) {
                    sourceFile = folder->url();
                    sourceFile.addPath( sFile );
                }
                
                tfiles += sourceFile;
                kDebug(9042) << "..........Adding:" << sourceFile << sFile << folder->url();
            }
            
            setTargetFiles(targetItem, tfiles);
        }
    } else {
        folder->cleanupBuildFolders(QList<Subdirectory>());
        folder->cleanupTargets(QList<CMakeTarget>());
    }
    reloadFiles(folder);

    return folderList;
}

bool containsFile(const KUrl& file, const QList<ProjectFileItem*>& tfiles)
{
    foreach(ProjectFileItem* f, tfiles) {
        if(f->url()==file)
            return true;
    }
    return false;
}

void CMakeManager::setTargetFiles(ProjectTargetItem* target, const KUrl::List& files)
{
    QList<ProjectFileItem*> tfiles = target->fileList();
    foreach(ProjectFileItem* file, tfiles) {
        if(!files.contains(file->url()))
            target->removeRow(file->row());
    }
    
    tfiles = target->fileList(); //We need to recreate the list without the removed items
    foreach(const KUrl& file, files) {
        if(!containsFile(file, tfiles))
            new KDevelop::ProjectFileItem( target->project(), file, target );   
    }
}

QList<KDevelop::ProjectTargetItem*> CMakeManager::targets() const
{
    QList<KDevelop::ProjectTargetItem*> ret;
    foreach(IProject* p, m_watchers.keys())
    {
        ret+=p->projectItem()->targetList();
    }
    return ret;
}


KUrl::List CMakeManager::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    CMakeFolderItem* folder=0;
//     kDebug(9042) << "Querying inc dirs for " << item;
    while(!folder && item)
    {
        folder = dynamic_cast<CMakeFolderItem*>( item );
        item = item->parent();
//         kDebug(9042) << "Looking for a folder: " << (folder ? folder->url() : KUrl()) << item;
    }
    Q_ASSERT(folder);

//     kDebug(9042) << "Include directories! -- before" << folder->includeDirectories();
    KUrl::List l = resolveSystemDirs(folder->project(), folder->includeDirectories());
//     kDebug(9042) << "Include directories!" << l;
    return l;
}

QHash< QString, QString > CMakeManager::defines(KDevelop::ProjectBaseItem *item ) const
{
    CMakeFolderItem* folder=0;
    kDebug(9042) << "Querying defines dirs for " << item;
    while(!folder)
    {
        folder = dynamic_cast<CMakeFolderItem*>( item );
        item = item->parent();
//         kDebug(9042) << "Looking for a folder: " << folder << item;
    }
    Q_ASSERT(folder);

    return folder->definitions();
}

KDevelop::IProjectBuilder * CMakeManager::builder(KDevelop::ProjectFolderItem *) const
{
    IPlugin* i = core()->pluginController()->pluginForExtension( "org.kdevelop.IProjectBuilder", "KDevCMakeBuilder");
    Q_ASSERT(i);
    KDevelop::IProjectBuilder* _builder = i->extension<KDevelop::IProjectBuilder>();
    Q_ASSERT(_builder );
    return _builder ;
}

/*void CMakeProjectManager::parseOnly(KDevelop::IProject* project, const KUrl &url)
{
    kDebug(9042) << "Looking for" << url << " to regenerate";

    KUrl cmakeListsPath(url);
    cmakeListsPath.addPath("CMakeLists.txt");

    VariableMap *vm=&m_varsPerProject[project];
    MacroMap *mm=&m_macrosPerProject[project];

    CMakeFileContent f = CMakeListsParser::readCMakeFile(cmakeListsPath.toLocalFile());
    if(f.isEmpty())
    {
        kDebug() << "There is no" << cmakeListsPath;
        return;
    }

    QString currentBinDir=KUrl::relativeUrl(project->projectItem()->url(), url);
    vm->insert("CMAKE_CURRENT_BINARY_DIR", QStringList(vm->value("CMAKE_BINARY_DIR")[0]+currentBinDir));
    vm->insert("CMAKE_CURRENT_LIST_FILE", QStringList(cmakeListsPath.toLocalFile(KUrl::RemoveTrailingSlash)));
    vm->insert("CMAKE_CURRENT_LIST_DIR", QStringList(url.toLocalFile(KUrl::RemoveTrailingSlash)));
    vm->insert("CMAKE_CURRENT_SOURCE_DIR", QStringList(url.toLocalFile(KUrl::RemoveTrailingSlash)));
    CMakeProjectVisitor v(url.toLocalFile(), missingtopcontext);
    v.setCacheValues(m_projectCache[project]);
    v.setVariableMap(vm);
    v.setMacroMap(mm);
    v.setModulePath(m_modulePathPerProject[project]);
    v.walk(f, 0);
    vm->remove("CMAKE_CURRENT_LIST_FILE");
    vm->remove("CMAKE_CURRENT_LIST_DIR");
    vm->remove("CMAKE_CURRENT_SOURCE_DIR");
    vm->remove("CMAKE_CURRENT_BINARY_DIR");
}*/

bool CMakeManager::reload(KDevelop::ProjectFolderItem* folder)
{
    if(isReloading(folder->project()))
        return false;
    
    CMakeFolderItem* item=dynamic_cast<CMakeFolderItem*>(folder);
    if ( !item ) {
        ProjectBaseItem* it = folder;
        while(!item && it->parent()) {
            it = it->parent();
            item = dynamic_cast<CMakeFolderItem*>(it);
        }
    }

    reimport(item);
    return true;
}

void CMakeManager::reimport(KDevelop::ProjectFolderItem* fi)
{
    KJob *job=createImportJob(fi);
    job->setProperty("projectitem", qVariantFromValue(fi));
    
    QMutexLocker locker(&m_busyProjectsMutex);
    m_busyProjects += fi;
    locker.unlock();
    
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( reimportDone( KJob* ) ) );
    ICore::self()->runController()->registerJob( job );
}

void CMakeManager::reimportDone(KJob* job)
{
    QMutexLocker locker(&m_busyProjectsMutex);
    ProjectFolderItem* it = job->property("projectitem").value<KDevelop::ProjectFolderItem*>();
    
    Q_ASSERT(m_busyProjects.contains(it));
    m_busyProjects.remove(it);
}

bool CMakeManager::isReloading(IProject* p)
{
    if(!p->isReady())
        return true;
    
    QMutexLocker locker(&m_busyProjectsMutex);
    foreach(KDevelop::ProjectFolderItem* it, m_busyProjects) {
        if(it->project()==p)
            return true;
    }
    return false;
}

void CMakeManager::deletedWatched(const QString& path)
{
    KUrl dirurl(path);
    IProject* p=0;
    if(m_busyProjects.isEmpty())
        p=ICore::self()->projectController()->findProjectForUrl(dirurl);
    else
        QMetaObject::invokeMethod(this, "deletedWatched", Qt::QueuedConnection, Q_ARG(QString, path));
    
    if(p) {
        if(!isReloading(p)) {
            dirurl.adjustPath(KUrl::AddTrailingSlash);
            if(p->folder()==dirurl) {
                ICore::self()->projectController()->closeProject(p);
            } else {
                KUrl url(path);
                
                if(path.endsWith("/CMakeLists.txt")) {
                    QList<ProjectFolderItem*> folders = p->foldersForUrl(url.upUrl());
                    foreach(ProjectFolderItem* folder, folders) 
                        reload(folder);
                    
                } else {
                    QMutexLocker locker(&m_busyProjectsMutex);
                    m_busyProjects += p->projectItem();
                    locker.unlock();
                    
                    qDeleteAll(p->itemsForUrl(url));
                    
                    locker.relock();
                    m_busyProjects -= p->projectItem();
                }
            }
        } else {
            QMetaObject::invokeMethod(this, "deletedWatched", Qt::QueuedConnection, Q_ARG(QString, path));
        }
    }
}

void CMakeManager::dirtyFile(const QString & dirty)
{
    const KUrl dirtyFile(dirty);
    IProject* p=ICore::self()->projectController()->findProjectForUrl(dirtyFile);

    kDebug() << "dirty FileSystem: " << dirty << (p ? isReloading(p) : 0);
    if(p && isReloading(p))
        return;
    
    if(p && dirtyFile.fileName() == "CMakeLists.txt")
    {
        QMutexLocker locker(&m_reparsingMutex); //Maybe we should have a mutex per project
        
        QList<ProjectFileItem*> files=p->filesForUrl(dirtyFile);
        kDebug(9032) << dirtyFile << "is dirty" << files.count();

        Q_ASSERT(files.count()==1);
        CMakeFolderItem *folderItem=static_cast<CMakeFolderItem*>(files.first()->parent());
        
#if 0
            KUrl relative=KUrl::relativeUrl(projectBaseUrl, dir);
            initializeProject(proj, dir);
            KUrl current=projectBaseUrl;
            QStringList subs=relative.toLocalFile().split("/");
            subs.append(QString());
            for(; !subs.isEmpty(); current.cd(subs.takeFirst()))
            {
                parseOnly(proj, current);
            }
#endif

        
        reload(folderItem);
    }
    else if(dirtyFile.fileName() == "CMakeCache.txt") {
        KUrl builddirUrl;
        IProject* p=0;
        //we first have to check from which project is this builddir
        foreach(KDevelop::IProject* pp, m_watchers.uniqueKeys()) {
            KUrl url = pp->buildSystemManager()->buildDirectory(pp->projectItem());
            if(dirtyFile.upUrl().equals(url, KUrl::CompareWithoutTrailingSlash)) {
                builddirUrl=url;
                p=pp;
            }
        }
        
        if(p) {
            p->reloadModel();
        }
    } else if(dirty.endsWith(".cmake"))
    {
        foreach(KDevelop::IProject* project, m_watchers.uniqueKeys())
        {
            if(m_watchers[project]->contains(dirty))
                project->reloadModel();
        }
    }
    else if(p && QFileInfo(dirty).isDir())
    {
        QList<ProjectFolderItem*> folders=p->foldersForUrl(dirty);
        Q_ASSERT(folders.isEmpty() || folders.size()==1);
        
        if(!folders.isEmpty()) {
            QMutexLocker locker(&m_busyProjectsMutex);
            m_busyProjects += folders.first();
            locker.unlock();
            
            reloadFiles(folders.first());
            
            locker.relock();
            m_busyProjects.remove(folders.first());
            locker.unlock();
        }
    }
}

void CMakeManager::reloadFiles(ProjectFolderItem* item)
{
    Q_ASSERT(isReloading(item->project()));
    
    QDir d(item->url().toLocalFile());
    if(!d.exists()) {
        kDebug() << "Trying to return a directory that doesn't exist:" << item->url();
        return;
    }
    
    QStringList entriesL = d.entryList( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QSet<QString> entries = filterFiles(entriesL);
    
    KUrl folderurl = item->url();
    
    kDebug() << "Reloading Directory!" << folderurl;
    
    //We look for removed elements
    for(int i=0; i<item->rowCount(); i++)
    {
        ProjectBaseItem* it=item->child(i);
        if(it->type()==ProjectBaseItem::Target || it->type()==ProjectBaseItem::ExecutableTarget || it->type()==ProjectBaseItem::LibraryTarget)
            continue;
        
        QString current=it->text();
        KUrl fileurl = folderurl;
        fileurl.addPath(current);
        
        if(!entries.contains(current))
        {
            qDeleteAll(item->project()->itemsForUrl(fileurl));
        }
        else {
            if(!it->url().equals(fileurl, KUrl::CompareWithoutTrailingSlash)) {
                it->setUrl(fileurl);
            }
            // reduce amount of checks done when looking for new items
            entries.remove(current);
        }
    }
    
    //We look for new elements
    foreach( const QString& entry, entries )
    {
        KUrl fileurl = folderurl;
        fileurl.addPath( entry );

        // existing entries should have been removed above already
        // disabled for performance reasons
//         Q_ASSERT( !item->hasFileOrFolder( entry ) );

        if( QFileInfo( fileurl.toLocalFile() ).isDir() )
        {
            fileurl.adjustPath(KUrl::AddTrailingSlash);
            ProjectFolderItem* pendingfolder = m_pending.take(fileurl);
            
            if(pendingfolder) {
                item->appendRow(pendingfolder);
            } else if(isCorrectFolder(fileurl, item->project())) {
                fileurl.adjustPath(KUrl::AddTrailingSlash);
                m_watchers[item->project()]->addDir(fileurl.toLocalFile(), KDirWatch::WatchFiles);
                reloadFiles(new ProjectFolderItem( item->project(), fileurl, item ));
            }
        }
        else
        {
            new KDevelop::ProjectFileItem( item->project(), fileurl, item );
        }
    }
}

bool CMakeManager::isCorrectFolder(const KUrl& url, IProject* p) const
{
    KUrl cache=url, missing=url;
    cache.addPath("CMakeCache.txt");
    missing.addPath(".kdev_ignore");
    
    bool ret = !QFile::exists(cache.toLocalFile()) && !QFile::exists(missing.toLocalFile());
    ret &= !CMake::allBuildDirs(p).contains(url.toLocalFile(KUrl::RemoveTrailingSlash));
    
    return ret;
}

QList< KDevelop::ProjectTargetItem * > CMakeManager::targets(KDevelop::ProjectFolderItem * folder) const
{
    return folder->targetList();
}

QString CMakeManager::name() const
{
    return "CMake";
}

KDevelop::ParseJob * CMakeManager::createParseJob(const KUrl &)
{
    return 0;
}

KDevelop::ILanguage * CMakeManager::language()
{
    return core()->languageController()->language(name());
}

KDevelop::ICodeHighlighting* CMakeManager::codeHighlighting() const
{
    return m_highlight;
}

ContextMenuExtension CMakeManager::contextMenuExtension( KDevelop::Context* context )
{
    if( context->type() != KDevelop::Context::ProjectItemContext )
        return IPlugin::contextMenuExtension( context );

    KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
    QList<KDevelop::ProjectBaseItem*> items = ctx->items();

    if( items.isEmpty() )
        return IPlugin::contextMenuExtension( context );

    m_clickedItems = items;
    ContextMenuExtension menuExt;
    if(items.count()==1 && dynamic_cast<DUChainAttatched*>(items.first()))
    {
        KAction* action = new KAction( i18n( "Jump to target definition" ), this );
        connect( action, SIGNAL( triggered() ), this, SLOT( jumpToDeclaration() ) );
        menuExt.addAction( ContextMenuExtension::ProjectGroup, action );
    }

    return menuExt;
}

void CMakeManager::jumpToDeclaration()
{
    DUChainAttatched* du=dynamic_cast<DUChainAttatched*>(m_clickedItems.first());
    if(du)
    {
        KTextEditor::Cursor c;
        KUrl url;
        {
            KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
            Declaration* decl = du->declaration().data();
            if(!decl)
                return;
            c = decl->rangeInCurrentRevision().start.textCursor();
            url = decl->url().toUrl();
        }

        ICore::self()->documentController()->openDocument(url, c);
    }
}

CacheValues CMakeManager::readCache(const KUrl &path) const
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

KDevelop::ProjectFolderItem* CMakeManager::addFolder( const KUrl& folder, KDevelop::ProjectFolderItem* parent)
{
    bool created = KDevelop::createFolder(folder);
    if(!created || !dynamic_cast<CMakeFolderItem*>(parent)) {
        return 0;
    }
    
    KUrl lists=parent->url();
    lists.addPath("CMakeLists.txt");
    
    QString relative=KUrl::relativeUrl(parent->url(), folder);

    kDebug() << "Adding folder " << parent->url() << " to " << folder << " as " << relative;
    Q_ASSERT(!relative.contains("/"));

    ApplyChangesWidget e;
    e.setCaption(relative);
    e.setInformation(i18n("Create a folder called '%1'.", relative));
    e.addDocuments(IndexedString(lists));

    e.document()->insertLine(e.document()->lines(), QString("add_subdirectory(%1)").arg(relative));

    if(e.exec())
    {
        bool saved=e.applyAllChanges();
        if(saved) { //If saved we create the folder then the CMakeLists.txt file
            KUrl newCMakeLists(folder);
            newCMakeLists.addPath("CMakeLists.txt");

            QFile f(newCMakeLists.toLocalFile());
            f.open(QIODevice::WriteOnly | QIODevice::Text);
            QTextStream out(&f);
            out << "\n";
        } else
            KMessageBox::error(0, i18n("KDevelop - CMake Support"),
                                  i18n("Could not save the change."));
    }
    return 0;
}

bool followUses(KTextEditor::Document* doc, RangeInRevision r, const QString& name, const KUrl& lists, bool add, const QString& replace)
{
    bool ret=false;
    QString txt=doc->text(r.castToSimpleRange().textRange());
    if(!add && txt.contains(name))
    {
        txt.replace(name, replace);
        doc->replaceText(r.castToSimpleRange().textRange(), txt);
        ret=true;
    }
    else
    {
        KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
        KDevelop::ReferencedTopDUContext topctx=DUChain::self()->chainForDocument(lists);
        QList<Declaration*> decls;
        for(int i=0; i<topctx->usesCount(); i++)
        {
            Use u = topctx->uses()[i];

            if(!r.contains(u.m_range))
                continue; //We just want the uses in the range, not the whole file

            Declaration* d=u.usedDeclaration(topctx);

            if(d && d->context()->topContext()->url().toUrl()==lists)
                decls += d;
        }

        if(add && decls.isEmpty())
        {
            doc->insertText(r.castToSimpleRange().textRange().start(), name);
            ret=true;
        }
        else foreach(Declaration* d, decls)
        {
            r.start=d->range().end;

            for(int lineNum = r.start.line; lineNum <= r.end.line; lineNum++)
            {
                int endParenIndex = doc->line(lineNum).indexOf(')');
                if(endParenIndex >= 0) {
                    r.end = CursorInRevision(lineNum, endParenIndex);
                    break;
                }
            }

            if(!r.isEmpty())
            {
                ret = ret || followUses(doc, r, name, lists, add, replace);
            }
        }
    }
    return ret;
}

QList<TargetFilePair> CMakeManager::getTargetFilesWithin(const QList<KDevelop::ProjectBaseItem*> &items) const
{
    QList<TargetFilePair> targetFiles;
    foreach (ProjectBaseItem* item, items)
    {
        if (ProjectFileItem* file = item->file())
        {
            //FIXME: ProjectFileItems should have a list of the targets they belong to
            QList<ProjectFileItem*> allItemsForUrl = file->project()->filesForUrl(file->url());
            foreach(ProjectFileItem* item, allItemsForUrl)
            {
                if (ProjectTargetItem* target = item->parent()->target())
                    targetFiles << TargetFilePair(target, item);
            }
        }
        else if (ProjectFolderItem* folder = item->folder())
            targetFiles += getTargetFilesWithin(folder->children());
    }
    return targetFiles;
}

QList<CMakeFolderItem*> CMakeManager::getCMakeFoldersWithin(const QList<KDevelop::ProjectBaseItem*> &items) const
{
    QList<CMakeFolderItem*> cmakeFolders;
    foreach (ProjectBaseItem* item, items)
    {
        switch (item->type())
        {
        case KDevelop::ProjectBaseItem::BuildFolder:
            cmakeFolders << static_cast<CMakeFolderItem*>(item);
            //let it drop
        case KDevelop::ProjectBaseItem::Folder:
            getCMakeFoldersWithin(item->children());
            break;
        }
    }
    return cmakeFolders;
}

bool CMakeManager::changesWidgetAddCMakeFolderRemovals(const QList<CMakeFolderItem*> &folders, ApplyChangesWidget* changesWidget)
{
    foreach(CMakeFolderItem* folder, folders)
    {
        KUrl lists = folder->url().upUrl();
        lists.addPath("CMakeLists.txt");

        changesWidget->addDocuments(IndexedString(lists));
        KTextEditor::Range range = folder->descriptor().range().castToSimpleRange().textRange();
        if (!changesWidget->document()->removeText(range))
            return false;
    }
    return true;
}

bool CMakeManager::changesWidgetAddTargetFileRemovals(const QList<TargetFilePair> &targetFiles, ApplyChangesWidget* changesWidget)
{
    foreach (TargetFilePair targetFile, targetFiles)
    {
        Q_ASSERT(targetFile.second->parent() == targetFile.first);

        CMakeFolderItem* folder = static_cast<CMakeFolderItem*>(targetFile.first->parent());

        DescriptorAttatched* desc = dynamic_cast<DescriptorAttatched*>(targetFile.first);
        RangeInRevision range = desc->descriptor().range();
        range.start = CursorInRevision(desc->descriptor().arguments.first().range().end);

        KUrl lists = folder->url();
        lists.addPath("CMakeLists.txt");

        changesWidget->addDocuments(IndexedString(lists));

        //FIXME: Not sure if this "find" extraction will cover all use cases
        QString find = KUrl::relativeUrl(folder->url(), targetFile.second->url());
        if (!followUses(changesWidget->document(), range, ' '+find, lists, false, QString()))
            return false;
    }
    return true;
}

bool CMakeManager::removeFilesAndFolders( QList<KDevelop::ProjectBaseItem*> items)
{
    //First do CMakeLists changes
    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(i18n("CMakeLists Changes"));
    changesWidget.setInformation(i18n("Remove files and folders from CMakeLists as follows:"));

    bool cmakeSuccessful = true;
    cmakeSuccessful &= changesWidgetAddTargetFileRemovals(getTargetFilesWithin(items), &changesWidget);
    cmakeSuccessful &= changesWidgetAddCMakeFolderRemovals(getCMakeFoldersWithin(items), &changesWidget);

    if (changesWidget.hasDocuments())
        cmakeSuccessful &= changesWidget.exec() && changesWidget.applyAllChanges();

    if (!cmakeSuccessful)
    {
        if (KMessageBox::questionYesNo( QApplication::activeWindow(),
                                        i18n("Changes to CMakeLists failed, abort file deletion?"),
                                        i18n("Error"))
            == KMessageBox::Yes)
        {
            return false;
        }
    }

    //Then delete the files/folders
    foreach(ProjectBaseItem* item, items)
    {
        Q_ASSERT(item->folder() || item->file());
        Q_ASSERT(!item->file() || !item->file()->parent()->target());

        if (!KDevelop::removeUrl(item->project(), item->url(), false))
            return false;
    }

    return true;
}

bool CMakeManager::removeFilesFromTargets( QList<TargetFilePair> targetFiles)
{
    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(i18n("CMakeLists Changes"));
    changesWidget.setInformation(i18n("Modify project targets as follows:"));

    if (targetFiles.size() &&
        changesWidgetAddTargetFileRemovals(targetFiles, &changesWidget) &&
        changesWidget.exec() &&
        changesWidget.applyAllChanges()) {
        return true;
    }

    return false;
}

//This is being called from ::parse() so we shouldn't make it block the ui
KDevelop::ProjectFileItem* CMakeManager::addFile( const KUrl& url, KDevelop::ProjectFolderItem* parent)
{
    KDevelop::ProjectFileItem* created = 0;
    if ( KDevelop::createFile(url) ) {
        created = new KDevelop::ProjectFileItem( parent->project(), url, parent );
    }
    return created;
}

bool CMakeManager::addFileToTarget( KDevelop::ProjectFileItem* it, KDevelop::ProjectTargetItem* target)
{
    Q_ASSERT(!it->url().isEmpty());
    
    QSet<QString> headerExt=QSet<QString>() << ".h" << ".hpp" << ".hxx";
    foreach(const QString& ext, headerExt)
    {
        if(it->url().fileName().toLower().endsWith(ext))
            return false;
    }
    
    if(it->parent()==target)
        return true; //It already is in the target

    CMakeFolderItem* folder=static_cast<CMakeFolderItem*>(target->parent());

    DescriptorAttatched* desc=dynamic_cast<DescriptorAttatched*>(target);
    RangeInRevision r=desc->descriptor().range();
    r.start=CursorInRevision(desc->descriptor().arguments.first().range().end);

    KUrl lists=folder->url();
    lists.addPath("CMakeLists.txt");

    ApplyChangesWidget e;
    e.setCaption(it->fileName());
    e.setInformation(i18n("Add a file called '%1' to target '%2'.", it->fileName(), target->text()));
    e.addDocuments(IndexedString(lists));

    QString filename=KUrl::relativeUrl(folder->url(), it->url());
    if(filename.startsWith("./"))
        filename=filename.right(filename.size()-2);
    bool ret=followUses(e.document(), r, ' '+filename, lists, true, QString());

    if(ret && e.exec())
        ret=e.applyAllChanges();
    if(!ret)
            KMessageBox::error(0, i18n("KDevelop - CMake Support"),
                                  i18n("Cannot save the change."));
    return ret;
}

QWidget* CMakeManager::specialLanguageObjectNavigationWidget(const KUrl& url, const KDevelop::SimpleCursor& position)
{
    KDevelop::TopDUContextPointer top= TopDUContextPointer(KDevelop::DUChain::self()->chainForDocument(url));
    Declaration *decl=0;
    QString htmlDoc;
    if(top)
    {
        int useAt=top->findUseAt(top->transformToLocalRevision(position));
        if(useAt>=0)
        {
            Use u=top->uses()[useAt];
            decl=u.usedDeclaration(top->topContext());
        }
    }

    CMakeNavigationWidget* doc=0;
    if(decl)
    {
        doc=new CMakeNavigationWidget(top, decl);
    }
    else
    {
        const IDocument* d=ICore::self()->documentController()->documentForUrl(url);
        const KTextEditor::Document* e=d->textDocument();
        KTextEditor::Cursor start=position.textCursor(), end=position.textCursor(), step(0,1);
        for(QChar i=e->character(start); i.isLetter() || i=='_'; i=e->character(start-=step))
        {}
        start+=step;
        
        for(QChar i=e->character(end); i.isLetter() || i=='_'; i=e->character(end+=step))
        {}
        
        QString id=e->text(KTextEditor::Range(start, end));
        ICMakeDocumentation* docu=CMake::cmakeDocumentation();
        if( docu )
        {
            KSharedPtr<IDocumentation> desc=docu->description(id, url);
            if(!desc.isNull())
            {
                doc=new CMakeNavigationWidget(top, desc);
            }
        }
    }
    
    return doc;
}

QPair<QString, QString> CMakeManager::cacheValue(KDevelop::IProject* project, const QString& id) const
{
    QPair<QString, QString> ret;
    if(project==0 && !m_projectsData.keys().isEmpty())
    {
        project=m_projectsData.keys().first();
    }
    
//     kDebug() << "cache value " << id << project << (m_projectsData.contains(project) && m_projectsData[project].cache.contains(id));
    if(m_projectsData.contains(project) && m_projectsData[project].cache.contains(id))
    {
        const CacheEntry& e=m_projectsData[project].cache.value(id);
        ret.first=e.value;
        ret.second=e.doc;
    }
    return ret;
}


bool CMakeManager::renameFile(ProjectFileItem* it, const KUrl& newUrl)
{
    KUrl fileUrl = it->url();
    IProject *project = it->project();
    QList<ProjectFileItem*> files=it->project()->filesForUrl(fileUrl);
    
    QList<ProjectTargetItem*> targets;
    
    //We loop through all the files with the same url
    foreach(ProjectFileItem* file, files)
    {
        ProjectTargetItem* t=static_cast<ProjectBaseItem*>(file->parent())->target();
        if(t)
            targets+=t;
    }
        
    if(targets.isEmpty())
    {
        return KDevelop::renameUrl(project, fileUrl, newUrl);
    }
    
    ApplyChangesWidget e;
    e.setCaption(it->text());
    e.setInformation(i18n("Remove a file called '%1'.", it->text()));
    
    bool ret=false;
    foreach(ProjectTargetItem* target, targets)
    {
        CMakeFolderItem* folder=static_cast<CMakeFolderItem*>(target->parent());

        DescriptorAttatched* desc=dynamic_cast<DescriptorAttatched*>(target);
        RangeInRevision r=desc->descriptor().range();
        r.start=CursorInRevision(desc->descriptor().arguments.first().range().end);

        KUrl lists=folder->url();
        lists.addPath("CMakeLists.txt");
        e.addDocuments(IndexedString(lists));
        
        QString newName=KUrl::relativeUrl(fileUrl.upUrl(), newUrl);
        if(newName.startsWith("./"))
            newName.remove(0,2);
        bool hasChanges = followUses(e.document(), r, ' '+it->text(), lists, false, ' '+newName);
        ret = ret || hasChanges;
    }

    ret &= e.exec()==KDialog::Accepted;
    if(ret)
    {
        ret=e.applyAllChanges();
        ret = ret && KDevelop::renameUrl(project, fileUrl, newUrl);
    }

    return ret;
}

bool CMakeManager::renameFolder(ProjectFolderItem* _it, const KUrl& newUrl)
{
    if(_it->type()!=KDevelop::ProjectBaseItem::BuildFolder)
    {
        return KDevelop::renameUrl(_it->project(), _it->url(), newUrl);
    }
    
    CMakeFolderItem* it=static_cast<CMakeFolderItem*>(_it);
    KUrl lists=it->formerParent()->url();
    lists.addPath("CMakeLists.txt");
    QString newName=KUrl::relativePath(lists.upUrl().path(), newUrl.path());
    if(newName.startsWith("./"))
        newName.remove(0,2);
    
    KUrl url = it->url();
    IProject* project = it->project();

    ApplyChangesWidget e;
    e.setCaption(it->text());
    e.setInformation(i18n("Rename a folder called '%1'.", it->text()));
    e.addDocuments(IndexedString(lists));

    KTextEditor::Range r=it->descriptor().argRange().castToSimpleRange().textRange();
    kDebug(9042) << "For " << lists << " rename " << r;
    
    bool ret = e.document()->replaceText(r, newName);
    
    ret &= e.exec() == QDialog::Accepted;
    ret &= KDevelop::renameUrl(project, url, newUrl);
    ret &= e.applyAllChanges();
    return ret;
}

void CMakeManager::projectClosing(IProject* p)
{
    m_projectsData.remove(p); 
    m_watchers.remove(p);
}

#include "cmakemanager.moc"
