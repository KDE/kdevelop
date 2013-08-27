/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007-2013 Aleix Pol <aleixpol@kde.org>
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
#include "cmakeedit.h"

#include <QDir>
#include <QThread>
#include <QFileSystemWatcher>
#include <QTimer>

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
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
#include <interfaces/iruncontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <interfaces/idocumentation.h>
#include <util/environmentgrouplist.h>
#include <language/highlighting/codehighlighting.h>
#include <project/projectmodel.h>
#include <project/helper.h>
#include <project/interfaces/iprojectbuilder.h>
#include <language/codecompletion/codecompletion.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/use.h>
#include <language/duchain/duchain.h>

#include "cmakenavigationwidget.h"
#include "cmakecachereader.h"
#include "cmakecodecompletionmodel.h"
#include <generationexpressionsolver.h>
#include "icmakedocumentation.h"

#ifdef CMAKEDEBUGVISITOR
#include "cmakedebugvisitor.h"
#endif

#include "ui_cmakepossibleroots.h"
#include "cmakemodelitems.h"
#include "cmakeprojectdata.h"
#include "cmakecommitchangesjob.h"
#include "cmakeimportjob.h"
#include "cmakeutils.h"

Q_DECLARE_METATYPE(KDevelop::IProject*);

using namespace KDevelop;

K_PLUGIN_FACTORY(CMakeSupportFactory, registerPlugin<CMakeManager>(); )
K_EXPORT_PLUGIN(CMakeSupportFactory(KAboutData("kdevcmakemanager","kdevcmake", ki18n("CMake Manager"), "0.1", ki18n("Support for managing CMake projects"), KAboutData::License_GPL)))

const QString DIALOG_CAPTION = i18n("KDevelop - CMake Support");

CMakeManager::CMakeManager( QObject* parent, const QVariantList& )
    : KDevelop::IPlugin( CMakeSupportFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ILanguageSupport )
    KDEV_USE_EXTENSION_INTERFACE( ICMakeManager)

    if (hasError()) {
        return;
    }

    m_highlight = new KDevelop::CodeHighlighting(this);

    new CodeCompletion(this, new CMakeCodeCompletionModel(this), name());
    
    connect(ICore::self()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)), SLOT(projectClosing(KDevelop::IProject*)));

    m_fileSystemChangeTimer = new QTimer(this);
    m_fileSystemChangeTimer->setSingleShot(true);
    m_fileSystemChangeTimer->setInterval(100);
    connect(m_fileSystemChangeTimer,SIGNAL(timeout()),SLOT(filesystemBuffererTimeout()));
}

bool CMakeManager::hasError() const
{
    return KStandardDirs::findExe("cmake").isEmpty();
}

QString CMakeManager::errorDescription() const
{
    return hasError() ? i18n("cmake is not installed") : QString();
}

CMakeManager::~CMakeManager()
{}

KUrl CMakeManager::buildDirectory(KDevelop::ProjectBaseItem *item) const
{
    CMakeFolderItem *fi=dynamic_cast<CMakeFolderItem*>(item);
    KUrl ret;
    ProjectBaseItem* parent = fi ? fi->formerParent() : item->parent();
    if (parent)
        ret=buildDirectory(parent);
    else
        ret=CMake::currentBuildDir(item->project());
    
    if(fi)
        ret.addPath(fi->buildDir());
    return ret;
}

KDevelop::ProjectFolderItem* CMakeManager::import( KDevelop::IProject *project )
{
    kDebug(9042) << "== migrating cmake settings";
    CMake::attemptMigrate(project);
    kDebug(9042) << "== completed cmake migration";

    kDebug(9042) << "== updating cmake settings from model";
    int buildDirCount = CMake::buildDirCount(project);
    for( int i = 0; i < buildDirCount; ++i )
        CMake::updateConfig( project, i );
    kDebug(9042) << "== completed updating cmake settings";

    KUrl cmakeInfoFile(project->projectFileUrl());
    cmakeInfoFile = cmakeInfoFile.upUrl();
    cmakeInfoFile.addPath("CMakeLists.txt");

    KUrl folderUrl=project->folder();
    kDebug(9042) << "file is" << cmakeInfoFile.toLocalFile();

    if ( !cmakeInfoFile.isLocalFile() )
    {
        kWarning() << "error. not a local file. CMake support doesn't handle remote projects";
        return 0;
    }
    
    if(CMake::hasProjectRootRelative(project))
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

    CMakeFolderItem* rootItem = new CMakeFolderItem(project, project->folder(), QString(), 0 );
    QFileSystemWatcher* w = new QFileSystemWatcher(project);
    w->setObjectName(project->name()+"_ProjectWatcher");
    connect(w, SIGNAL(fileChanged(QString)), SLOT(dirtyFile(QString)));
    connect(w, SIGNAL(directoryChanged(QString)), SLOT(directoryChanged(QString)));
    m_watchers[project] = w;
    
    KUrl cachefile=CMake::currentBuildDir(project);
    if( cachefile.isEmpty() ) {
        CMake::checkForNeedingConfigure(project);
    } else {
        cachefile.addPath("CMakeCache.txt");
        w->addPath(cachefile.toLocalFile());
    }
    
    Q_ASSERT(rootItem->rowCount()==0);
    return rootItem;
}

QList<ProjectFolderItem*> CMakeManager::parse(ProjectFolderItem*)
{ return QList< ProjectFolderItem* >(); }


KJob* CMakeManager::createImportJob(ProjectFolderItem* dom)
{
    KJob* job = new CMakeImportJob(dom, this);
    connect(job, SIGNAL(finished(KJob*)), SLOT(importFinished(KJob*)));
    return job;
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
    IProject* project = item->project();
//     kDebug(9042) << "Querying inc dirs for " << item;
    while(item)
    {
        if(CompilationDataAttached* includer = dynamic_cast<CompilationDataAttached*>( item )) {
            QStringList dirs = includer->includeDirectories(item);
            //Here there's the possibility that it might not be a target. We should make sure that's not the case
            return CMake::resolveSystemDirs(project, processGeneratorExpression(dirs, project, dynamic_cast<ProjectTargetItem*>(item)));
        }
        item = item->parent();
//         kDebug(9042) << "Looking for an includer: " << item;
    }
    // No includer found, so no include-directories to be returned;
    return KUrl::List();
}

QHash< QString, QString > CMakeManager::defines(KDevelop::ProjectBaseItem *item ) const
{
    CompilationDataAttached* att=0;
    ProjectBaseItem* it=item;
//     kDebug(9042) << "Querying defines for " << item << dynamic_cast<ProjectTargetItem*>(item);
    while(!att && item)
    {
        att = dynamic_cast<CompilationDataAttached*>( item );
        it = item;
        item = item->parent();
//         kDebug(9042) << "Looking for a folder: " << folder << item;
    }
    if( !att ) {
        // Not a CMake folder, so no defines to be returned;
        return QHash<QString,QString>();
    }

    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>(it);
    CMakeDefinitions defs = att->definitions(folder ? folder->formerParent() : dynamic_cast<CMakeFolderItem*>(item));
    //qDebug() << "lalala" << defs << it->url();
    return defs;
}

KDevelop::IProjectBuilder * CMakeManager::builder() const
{
    IPlugin* i = core()->pluginController()->pluginForExtension( "org.kdevelop.IProjectBuilder", "KDevCMakeBuilder");
    Q_ASSERT(i);
    KDevelop::IProjectBuilder* _builder = i->extension<KDevelop::IProjectBuilder>();
    Q_ASSERT(_builder );
    return _builder ;
}

bool CMakeManager::reload(KDevelop::ProjectFolderItem* folder)
{
    kDebug(9032) << "reloading" << folder->url();
    if(isReloading(folder->project()))
        return false;
    
    CMakeFolderItem* fi = dynamic_cast<CMakeFolderItem*>(folder);
    for(ProjectBaseItem* it = folder; !fi && it->parent();) {
        it = it->parent();
        fi = dynamic_cast<CMakeFolderItem*>(it);
    }
    Q_ASSERT(fi && "at least the root item should be a CMakeFolderItem");

    m_busyProjects += fi->project();

    KJob *job=createImportJob(fi);
    connect(job, SIGNAL(result(KJob*)), SLOT(importFinished(KJob*)));
    ICore::self()->runController()->registerJob( job );
    return true;
}

void CMakeManager::importFinished(KJob* j)
{
    CMakeImportJob* job = qobject_cast<CMakeImportJob*>(j);
    Q_ASSERT(job);
    m_busyProjects.remove(job->project());
    *m_projectsData[job->project()] = job->projectData();
}

bool CMakeManager::isReloading(IProject* p)
{
    Q_ASSERT(p);
    return !p->isReady() || m_busyProjects.contains(p);
}

void CMakeManager::deletedWatchedDirectory(IProject* p, const KUrl& dir)
{
    if(p->folder().equals(dir, KUrl::CompareWithoutTrailingSlash)) {
        ICore::self()->projectController()->closeProject(p);
    } else {
        if(dir.fileName()=="CMakeLists.txt") {
            QList<ProjectFolderItem*> folders = p->foldersForUrl(dir.upUrl());
            foreach(ProjectFolderItem* folder, folders)
                reload(folder);
        } else {
            qDeleteAll(p->itemsForUrl(dir));
        }
    }
}

void CMakeManager::directoryChanged(const QString& dir)
{
    m_fileSystemChangedBuffer << dir;
    m_fileSystemChangeTimer->start();
}

void CMakeManager::filesystemBuffererTimeout()
{
    Q_FOREACH(const QString& file, m_fileSystemChangedBuffer) {
        realDirectoryChanged(file);
    }
    m_fileSystemChangedBuffer.clear();
}

void CMakeManager::realDirectoryChanged(const QString& dir)
{
    KUrl path(dir);
    IProject* p=ICore::self()->projectController()->findProjectForUrl(dir);
    if(!p || isReloading(p)) {
        if(p) {
            m_fileSystemChangedBuffer << dir;
            m_fileSystemChangeTimer->start();
        }
        return;
    }
    
    if(!QFile::exists(dir)) {
        path.adjustPath(KUrl::AddTrailingSlash);
        deletedWatchedDirectory(p, path);
    } else
        dirtyFile(dir);
}

void CMakeManager::dirtyFile(const QString & dirty)
{
    const KUrl dirtyFile(dirty);
    IProject* p=ICore::self()->projectController()->findProjectForUrl(dirtyFile);

    kDebug() << "dirty FileSystem: " << dirty << (p ? isReloading(p) : 0);
    if(p)
    {
        if(dirtyFile.fileName() == "CMakeLists.txt") {
            QList<ProjectFileItem*> files=p->filesForUrl(dirtyFile);;

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
        else if(QFileInfo(dirty).isDir() && !isReloading(p))
        {
            QList<ProjectFolderItem*> folders=p->foldersForUrl(dirty);
            Q_ASSERT(folders.isEmpty() || folders.size()==1);
            
            if(!folders.isEmpty()) {
                CMakeCommitChangesJob* job = new CMakeCommitChangesJob(dirtyFile, this, p);
                job->start();
            }
        }
    }
    else if(dirtyFile.fileName()=="CMakeCache.txt")
    {
        //we first have to check from which project is this builddir
        foreach(KDevelop::IProject* pp, m_watchers.uniqueKeys()) {
            KUrl buildDir = CMake::currentBuildDir(pp);
            if(dirtyFile.upUrl().equals(buildDir, KUrl::CompareWithoutTrailingSlash)) {
                reload(pp->projectItem());
            }
        }
    }
    else if(dirty.endsWith(".cmake"))
    {
        foreach(KDevelop::IProject* project, m_watchers.uniqueKeys())
        {
            if(m_watchers[project]->files().contains(dirty))
                reload(project->projectItem());
        }
    }
}

QList< KDevelop::ProjectTargetItem * > CMakeManager::targets(KDevelop::ProjectFolderItem * folder) const
{
    return folder->targetList();
}

QString CMakeManager::name() const
{
    return "CMake";
}

KDevelop::ParseJob * CMakeManager::createParseJob(const IndexedString &/*url*/)
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
        KAction* action = new KAction( i18n( "Jump to Target Definition" ), this );
        connect( action, SIGNAL(triggered()), this, SLOT(jumpToDeclaration()) );
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
            KDevelop::DUChainReadLocker lock;
            Declaration* decl = du->declaration().data();
            if(!decl)
                return;
            c = decl->rangeInCurrentRevision().start.textCursor();
            url = decl->url().toUrl();
        }

        ICore::self()->documentController()->openDocument(url, c);
    }
}

bool CMakeManager::moveFilesAndFolders(const QList< ProjectBaseItem* > &items, ProjectFolderItem* toFolder)
{
    using namespace CMakeEdit;

    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(DIALOG_CAPTION);
    changesWidget.setInformation(i18n("Move files and folders within CMakeLists as follows:"));

    bool cmakeSuccessful = true;
    CMakeFolderItem *nearestCMakeFolderItem = nearestCMakeFolder(toFolder);
    IProject* project=toFolder->project();
    
    KUrl::List movedUrls;
    KUrl::List oldUrls;
    foreach(ProjectBaseItem *movedItem, items)
    {
        QList<ProjectBaseItem*> dirtyItems = cmakeListedItemsAffectedByUrlChange(project, movedItem->url());
        KUrl movedItemNewUrl = toFolder->url();
        movedItemNewUrl.addPath(movedItem->baseName());
        if (movedItem->folder())
            movedItemNewUrl.adjustPath(KUrl::AddTrailingSlash);
        foreach(ProjectBaseItem* dirtyItem, dirtyItems)
        {
            KUrl dirtyItemNewUrl = afterMoveUrl(dirtyItem->url(), movedItem->url(), movedItemNewUrl);
            if (CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>(dirtyItem))
            {
                cmakeSuccessful &= changesWidgetRemoveCMakeFolder(folder, &changesWidget);
                cmakeSuccessful &= changesWidgetAddFolder(dirtyItemNewUrl, nearestCMakeFolderItem, &changesWidget);
            }
            else if (dirtyItem->parent()->target())
            {
                cmakeSuccessful &= changesWidgetMoveTargetFile(dirtyItem, dirtyItemNewUrl, &changesWidget);
            }
        }
        
        oldUrls += movedItem->url();
        movedUrls += movedItemNewUrl;
    }

    if (changesWidget.hasDocuments() && cmakeSuccessful)
        cmakeSuccessful &= changesWidget.exec() && changesWidget.applyAllChanges();

    if (!cmakeSuccessful)
    {
        if (KMessageBox::questionYesNo( QApplication::activeWindow(),
                                        i18n("Changes to CMakeLists failed, abort move?"),
                                        DIALOG_CAPTION ) == KMessageBox::Yes)
            return false;
    }

    KUrl::List::const_iterator it1=oldUrls.constBegin(), it1End=oldUrls.constEnd();
    KUrl::List::const_iterator it2=movedUrls.constBegin();
    Q_ASSERT(oldUrls.size()==movedUrls.size());
    for(; it1!=it1End; ++it1, ++it2)
    {
        if (!KDevelop::renameUrl(project, *it1, *it2))
            return false;
        
        QList<ProjectBaseItem*> renamedItems = project->itemsForUrl(*it2);
        bool dir = QFileInfo(it2->toLocalFile()).isDir();
        foreach(ProjectBaseItem* item, renamedItems) {
            if(dir)
                emit folderRenamed(*it1, item->folder());
            else
                emit fileRenamed(*it1, item->file());
        }
    }

    return true;
}

bool CMakeManager::copyFilesAndFolders(const KUrl::List &items, KDevelop::ProjectFolderItem* toFolder)
{
    IProject* project = toFolder->project();
    foreach(const KUrl& url, items) {
        if (!KDevelop::copyUrl(project, url, toFolder->url()))
            return false;
    }

    return true;
}

bool CMakeManager::removeFilesAndFolders(const QList<KDevelop::ProjectBaseItem*> &items)
{
    using namespace CMakeEdit;

    IProject* p = 0;
    QList<QUrl> urls;
    foreach(ProjectBaseItem* item, items)
    {
        Q_ASSERT(item->folder() || item->file());
        Q_ASSERT(!item->file() || !item->file()->parent()->target());

        urls += item->url();
        if(!p)
            p = item->project();
    }

    //First do CMakeLists changes
    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(DIALOG_CAPTION);
    changesWidget.setInformation(i18n("Remove files and folders from CMakeLists as follows:"));

    bool cmakeSuccessful = changesWidgetRemoveItems(cmakeListedItemsAffectedByItemsChanged(items), &changesWidget);

    if (changesWidget.hasDocuments() && cmakeSuccessful)
        cmakeSuccessful &= changesWidget.exec() && changesWidget.applyAllChanges();

    if (!cmakeSuccessful)
    {
        if (KMessageBox::questionYesNo( QApplication::activeWindow(),
                                        i18n("Changes to CMakeLists failed, abort deletion?"),
                                        DIALOG_CAPTION ) == KMessageBox::Yes)
            return false;
    }

    bool ret = true;
    //Then delete the files/folders
    foreach(const QUrl& file, urls)
    {
        ret &= KDevelop::removeUrl(p, file, QDir(file.toLocalFile()).exists());
    }

    return ret;
}

bool CMakeManager::removeFilesFromTargets(const QList<ProjectFileItem*> &files)
{
    using namespace CMakeEdit;

    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(DIALOG_CAPTION);
    changesWidget.setInformation(i18n("Modify project targets as follows:"));

    if (!files.isEmpty() &&
        changesWidgetRemoveFilesFromTargets(files, &changesWidget) &&
        changesWidget.exec() &&
        changesWidget.applyAllChanges()) {
        return true;
    }
    return false;
}

ProjectFolderItem* CMakeManager::addFolder(const KUrl& folder, ProjectFolderItem* parent)
{
    using namespace CMakeEdit;

    CMakeFolderItem *cmakeParent = nearestCMakeFolder(parent);
    if(!cmakeParent)
        return 0;

    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(DIALOG_CAPTION);
    changesWidget.setInformation(i18n("Create folder '%1':",
                                      folder.fileName(KUrl::IgnoreTrailingSlash)));

    changesWidgetAddFolder(folder, cmakeParent, &changesWidget);

    if(changesWidget.exec() && changesWidget.applyAllChanges())
    {
        if(KDevelop::createFolder(folder)) { //If saved we create the folder then the CMakeLists.txt file
            KUrl newCMakeLists(folder);
            newCMakeLists.addPath("CMakeLists.txt");
            KDevelop::createFile( newCMakeLists );
        } else
            KMessageBox::error(0, i18n("Could not save the change."),
                                  DIALOG_CAPTION);
    }

    return 0;
}

KDevelop::ProjectFileItem* CMakeManager::addFile( const KUrl& url, KDevelop::ProjectFolderItem* parent)
{
    KDevelop::ProjectFileItem* created = 0;
    if ( KDevelop::createFile(url) ) {
        QList< ProjectFileItem* > files = parent->project()->filesForUrl(url);
        if(!files.isEmpty())
            created = files.first();
        else
            created = new KDevelop::ProjectFileItem( parent->project(), url, parent );
    }
    return created;
}

bool CMakeManager::addFilesToTarget(const QList< ProjectFileItem* > &_files, ProjectTargetItem* target)
{
    using namespace CMakeEdit;

    const QSet<QString> headerExt = QSet<QString>() << ".h" << ".hpp" << ".hxx";
    QList< ProjectFileItem* > files = _files;
    for (int i = files.count() - 1; i >= 0; --i)
    {
        QString fileName = files[i]->fileName();
        QString fileExt = fileName.mid(fileName.lastIndexOf('.'));
        QList<ProjectBaseItem*> sameUrlItems = files[i]->project()->itemsForUrl(files[i]->url());
        if (headerExt.contains(fileExt))
            files.removeAt(i);
        else foreach(ProjectBaseItem* item, sameUrlItems)
        {
            if (item->parent() == target)
            {
                files.removeAt(i);
                break;
            }
        }
    }

    if(files.isEmpty())
        return true;

    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(DIALOG_CAPTION);
    changesWidget.setInformation(i18n("Modify target '%1' as follows:", target->baseName()));

    bool success = changesWidgetAddFilesToTarget(files, target, &changesWidget) &&
                   changesWidget.exec() &&
                   changesWidget.applyAllChanges();

    if(!success)
        KMessageBox::error(0, i18n("CMakeLists changes failed."), DIALOG_CAPTION);

    return success;
}

bool CMakeManager::renameFileOrFolder(ProjectBaseItem *item, const KUrl &newUrl)
{
    using namespace CMakeEdit;

    ApplyChangesWidget changesWidget;
    changesWidget.setCaption(DIALOG_CAPTION);
    changesWidget.setInformation(i18n("Rename '%1' to '%2':", item->text(),
                                      newUrl.fileName(KUrl::IgnoreTrailingSlash)));
    
    bool cmakeSuccessful = true, changedCMakeLists=false;
    IProject* project=item->project();
    KUrl oldUrl=item->url();
    if (item->file())
    {
        QList<ProjectBaseItem*> targetFiles = cmakeListedItemsAffectedByUrlChange(project, oldUrl);
        foreach(ProjectBaseItem* targetFile, targetFiles)
            cmakeSuccessful &= changesWidgetMoveTargetFile(targetFile, newUrl, &changesWidget);
    }
    else if (CMakeFolderItem *folder = dynamic_cast<CMakeFolderItem*>(item))
        cmakeSuccessful &= changesWidgetRenameFolder(folder, newUrl, &changesWidget);
    
    item->setUrl(newUrl);
    if (changesWidget.hasDocuments() && cmakeSuccessful) {
        changedCMakeLists = changesWidget.exec() && changesWidget.applyAllChanges();
        cmakeSuccessful &= changedCMakeLists;
    }
    
    if (!cmakeSuccessful)
    {
        if (KMessageBox::questionYesNo( QApplication::activeWindow(),
                                        i18n("Changes to CMakeLists failed, abort rename?"),
                                        DIALOG_CAPTION ) == KMessageBox::Yes)
            return false;
    }

    bool ret = KDevelop::renameUrl(project, oldUrl, newUrl);
    if(!ret) {
        item->setUrl(oldUrl);
    }
    return ret;
}

bool CMakeManager::renameFile(ProjectFileItem *item, const KUrl &newUrl)
{
    return renameFileOrFolder(item, newUrl);
}

bool CMakeManager::renameFolder(ProjectFolderItem* item, const KUrl &newUrl)
{
    return renameFileOrFolder(item, newUrl);
}

QWidget* CMakeManager::specialLanguageObjectNavigationWidget(const KUrl& url, const KDevelop::SimpleCursor& position)
{
    KDevelop::TopDUContextPointer top= TopDUContextPointer(KDevelop::DUChain::self()->chainForDocument(url));
    Declaration *decl=0;
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
    CMakeProjectData* data = m_projectsData[project];
    if(data && data->cache.contains(id))
    {
        const CacheEntry& e=data->cache.value(id);
        ret.first=e.value;
        ret.second=e.doc;
    }
    return ret;
}

void CMakeManager::projectClosing(IProject* p)
{
    delete m_projectsData.take(p); 
    delete m_watchers.take(p);
}

QStringList CMakeManager::processGeneratorExpression(const QStringList& expr, IProject* project, ProjectTargetItem* target) const
{
    QStringList ret;
    const CMakeProjectData* data = m_projectsData[project];
    GenerationExpressionSolver exec(data->properties);
    if(target)
        exec.setTargetName(target->text());

    exec.defineVariable("INSTALL_PREFIX", data->vm.value("CMAKE_INSTALL_PREFIX").join(QString()));
    for(QStringList::const_iterator it = expr.constBegin(), itEnd = expr.constEnd(); it!=itEnd; ++it) {
        QStringList val = exec.run(*it).split(';');
        ret += val;
    }
    return ret;
}

void CMakeManager::addPending(const KUrl& url, CMakeFolderItem* folder)
{
    m_pending.insert(url, folder);
}

CMakeFolderItem* CMakeManager::takePending(const KUrl& url)
{
    return m_pending.take(url);
}

void CMakeManager::addWatcher(IProject* p, const QString& path)
{
    m_watchers[p]->addPath(path);
}

CMakeProjectData CMakeManager::projectData(IProject* project)
{
    Q_ASSERT(QThread::currentThread() == project->thread());
    CMakeProjectData* data = m_projectsData[project];
    if(!data) {
        data = new CMakeProjectData;
        m_projectsData[project] = data;
    }
    return *data;
}
