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
#include "cmakeutils.h"
#include "cmakeprojectdata.h"
#include "duchain/cmakeparsejob.h"
#include "cmakeimportjsonjob.h"
#include "debug.h"
#include <projectmanagers/custommake/makefileresolver/makefileresolver.h>
#include "cmakecodecompletionmodel.h"
#include "cmakenavigationwidget.h"
#include "icmakedocumentation.h"

#include <QDir>
#include <QThread>
#include <QFileSystemWatcher>
#include <QTimer>
#include <qjsondocument.h>

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <QUrl>
#include <QAction>
#include <KMessageBox>
#include <ktexteditor/document.h>

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
#include <util/executecompositejob.h>
#include <language/highlighting/codehighlighting.h>
#include <project/projectmodel.h>
#include <project/helper.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/projectfiltermanager.h>
#include <language/codecompletion/codecompletion.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/use.h>
#include <language/duchain/duchain.h>
#include <QStandardPaths>

Q_DECLARE_METATYPE(KDevelop::IProject*);

using namespace KDevelop;

K_PLUGIN_FACTORY(CMakeSupportFactory, registerPlugin<CMakeManager>(); )

const QString DIALOG_CAPTION = i18n("KDevelop - CMake Support");

CMakeManager::CMakeManager( QObject* parent, const QVariantList& )
    : KDevelop::AbstractFileManagerPlugin( "kdevcmakemanager", parent )
    , m_filter( new ProjectFilterManager( this ) )
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

    connect(ICore::self()->projectController(), &IProjectController::projectClosing, this, &CMakeManager::projectClosing);

//     m_fileSystemChangeTimer = new QTimer(this);
//     m_fileSystemChangeTimer->setSingleShot(true);
//     m_fileSystemChangeTimer->setInterval(100);
//     connect(m_fileSystemChangeTimer,SIGNAL(timeout()),SLOT(filesystemBuffererTimeout()));
}

bool CMakeManager::hasError() const
{
    return QStandardPaths::findExecutable("cmake").isEmpty();
}

QString CMakeManager::errorDescription() const
{
    return hasError() ? i18n("cmake is not installed") : QString();
}

CMakeManager::~CMakeManager()
{}

Path CMakeManager::buildDirectory(KDevelop::ProjectBaseItem *item) const
{
//     CMakeFolderItem *fi=dynamic_cast<CMakeFolderItem*>(item);
//     Path ret;
//     ProjectBaseItem* parent = fi ? fi->formerParent() : item->parent();
//     if (parent)
//         ret=buildDirectory(parent);
//     else
//         ret=Path(CMake::currentBuildDir(item->project()));
//
//     if(fi)
//         ret.addPath(fi->buildDir());
//     return ret;
    return Path(CMake::currentBuildDir(item->project()));
}

KDevelop::ProjectFolderItem* CMakeManager::import( KDevelop::IProject *project )
{
    CMake::checkForNeedingConfigure(project);

    return AbstractFileManagerPlugin::import(project);
}

KJob* CMakeManager::createImportJob(ProjectFolderItem* item)
{
    auto project = item->project();

    QList<KJob*> jobs;

    // create the JSON file if it doesn't exist
    auto commandsFile = CMake::commandsFile(project);
    if (!QFileInfo(commandsFile.toLocalFile()).exists()) {
        qCDebug(CMAKE) << "couldn't find commands file:" << commandsFile << "- now trying to reconfigure";
        jobs << builder()->configure(project);
    }

    // parse the JSON file
    CMakeImportJob* job = new CMakeImportJob(project, this);
    connect(job, &CMakeImportJob::result, this, &CMakeManager::importFinished);
    jobs << job;

    // generate the file system listing
    jobs << KDevelop::AbstractFileManagerPlugin::createImportJob(item);

    ExecuteCompositeJob* composite = new ExecuteCompositeJob(this, jobs);
//     even if the cmake call failed, we want to load the project so that the project can be worked on
    composite->setAbortOnError(false);
    return composite;
}

// QList<ProjectFolderItem*> CMakeManager::parse(ProjectFolderItem*)
// { return QList< ProjectFolderItem* >(); }
//
//

QList<KDevelop::ProjectTargetItem*> CMakeManager::targets() const
{
    QList<KDevelop::ProjectTargetItem*> ret;
    foreach(IProject* p, m_projects.keys())
    {
        ret+=p->projectItem()->targetList();
    }
    return ret;
}

Path::List CMakeManager::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    return m_projects[item->project()].jsonData.files[item->path()].includes;
}

QHash<QString, QString> CMakeManager::defines(KDevelop::ProjectBaseItem *item ) const
{
    return m_projects[item->project()].jsonData.files[item->path()].defines;
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
    qCDebug(CMAKE) << "reloading" << folder->path();

    IProject* project = folder->project();
    if (!project->isReady())
        return false;

    KJob *job = createImportJob(folder);
    project->setReloadJob(job);
    ICore::self()->runController()->registerJob( job );
    return true;
}

void CMakeManager::importFinished(KJob* j)
{
    CMakeImportJob* job = qobject_cast<CMakeImportJob*>(j);
    Q_ASSERT(job);

    auto project = job->project();
    if (job->error() != 0) {
        qCDebug(CMAKE) << "Import failed for project" << project->name() << job->errorText();
        m_projects.remove(project);
    }

    qCDebug(CMAKE) << "Successfully imported project" << project->name();

    CMakeProjectData data;
    data.watcher->addPath(CMake::currentBuildDir(project).toLocalFile());
    data.jsonData = job->jsonData();
    connect(data.watcher.data(), &QFileSystemWatcher::fileChanged, this, &CMakeManager::dirtyFile);
    connect(data.watcher.data(), &QFileSystemWatcher::directoryChanged, this, &CMakeManager::dirtyFile);
    m_projects[job->project()] = data;
}

// void CMakeManager::deletedWatchedDirectory(IProject* p, const QUrl &dir)
// {
//     if(p->folder().equals(dir, QUrl::CompareWithoutTrailingSlash)) {
//         ICore::self()->projectController()->closeProject(p);
//     } else {
//         if(dir.fileName()=="CMakeLists.txt") {
//             QList<ProjectFolderItem*> folders = p->foldersForUrl(dir.upUrl());
//             foreach(ProjectFolderItem* folder, folders)
//                 reload(folder);
//         } else {
//             qDeleteAll(p->itemsForUrl(dir));
//         }
//     }
// }

// void CMakeManager::directoryChanged(const QString& dir)
// {
//     m_fileSystemChangedBuffer << dir;
//     m_fileSystemChangeTimer->start();
// }

// void CMakeManager::filesystemBuffererTimeout()
// {
//     Q_FOREACH(const QString& file, m_fileSystemChangedBuffer) {
//         realDirectoryChanged(file);
//     }
//     m_fileSystemChangedBuffer.clear();
// }

// void CMakeManager::realDirectoryChanged(const QString& dir)
// {
//     QUrl path(dir);
//     IProject* p=ICore::self()->projectController()->findProjectForUrl(dir);
//     if(!p || !p->isReady()) {
//         if(p) {
//             m_fileSystemChangedBuffer << dir;
//             m_fileSystemChangeTimer->start();
//         }
//         return;
//     }
//
//     if(!QFile::exists(dir)) {
//         path.adjustPath(QUrl::AddTrailingSlash);
//         deletedWatchedDirectory(p, path);
//     } else
//         dirtyFile(dir);
// }

QList< KDevelop::ProjectTargetItem * > CMakeManager::targets(KDevelop::ProjectFolderItem * folder) const
{
    return folder->targetList();
}

QString CMakeManager::name() const
{
    return languageName().str();
}

IndexedString CMakeManager::languageName()
{
    static IndexedString name("CMake");
    return name;
}

KDevelop::ParseJob * CMakeManager::createParseJob(const IndexedString &url)
{
    return new CMakeParseJob(url, this);
}

KDevelop::ICodeHighlighting* CMakeManager::codeHighlighting() const
{
    return m_highlight;
}

// ContextMenuExtension CMakeManager::contextMenuExtension( KDevelop::Context* context )
// {
//     if( context->type() != KDevelop::Context::ProjectItemContext )
//         return IPlugin::contextMenuExtension( context );
//
//     KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
//     QList<KDevelop::ProjectBaseItem*> items = ctx->items();
//
//     if( items.isEmpty() )
//         return IPlugin::contextMenuExtension( context );
//
//     m_clickedItems = items;
//     ContextMenuExtension menuExt;
//     if(items.count()==1 && dynamic_cast<DUChainAttatched*>(items.first()))
//     {
//         QAction * action = new QAction( i18n( "Jump to Target Definition" ), this );
//         connect( action, SIGNAL(triggered()), this, SLOT(jumpToDeclaration()) );
//         menuExt.addAction( ContextMenuExtension::ProjectGroup, action );
//     }
//
//     return menuExt;
// }
//
// void CMakeManager::jumpToDeclaration()
// {
//     DUChainAttatched* du=dynamic_cast<DUChainAttatched*>(m_clickedItems.first());
//     if(du)
//     {
//         KTextEditor::Cursor c;
//         QUrl url;
//         {
//             KDevelop::DUChainReadLocker lock;
//             Declaration* decl = du->declaration().data();
//             if(!decl)
//                 return;
//             c = decl->rangeInCurrentRevision().start();
//             url = decl->url().toUrl();
//         }
//
//         ICore::self()->documentController()->openDocument(url, c);
//     }
// }
//
// // TODO: Port to Path API
// bool CMakeManager::moveFilesAndFolders(const QList< ProjectBaseItem* > &items, ProjectFolderItem* toFolder)
// {
//     using namespace CMakeEdit;
//
//     ApplyChangesWidget changesWidget;
//     changesWidget.setCaption(DIALOG_CAPTION);
//     changesWidget.setInformation(i18n("Move files and folders within CMakeLists as follows:"));
//
//     bool cmakeSuccessful = true;
//     CMakeFolderItem *nearestCMakeFolderItem = nearestCMakeFolder(toFolder);
//     IProject* project=toFolder->project();
//
//     QList<QUrl> movedUrls;
//     QList<QUrl> oldUrls;
//     foreach(ProjectBaseItem *movedItem, items)
//     {
//         QList<ProjectBaseItem*> dirtyItems = cmakeListedItemsAffectedByUrlChange(project, movedItem->url());
//         QUrl movedItemNewUrl = toFolder->url();
//         movedItemNewUrl.addPath(movedItem->baseName());
//         if (movedItem->folder())
//             movedItemNewUrl.adjustPath(QUrl::AddTrailingSlash);
//         foreach(ProjectBaseItem* dirtyItem, dirtyItems)
//         {
//             QUrl dirtyItemNewUrl = afterMoveUrl(dirtyItem->url(), movedItem->url(), movedItemNewUrl);
//             if (CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>(dirtyItem))
//             {
//                 cmakeSuccessful &= changesWidgetRemoveCMakeFolder(folder, &changesWidget);
//                 cmakeSuccessful &= changesWidgetAddFolder(dirtyItemNewUrl, nearestCMakeFolderItem, &changesWidget);
//             }
//             else if (dirtyItem->parent()->target())
//             {
//                 cmakeSuccessful &= changesWidgetMoveTargetFile(dirtyItem, dirtyItemNewUrl, &changesWidget);
//             }
//         }
//
//         oldUrls += movedItem->url();
//         movedUrls += movedItemNewUrl;
//     }
//
//     if (changesWidget.hasDocuments() && cmakeSuccessful)
//         cmakeSuccessful &= changesWidget.exec() && changesWidget.applyAllChanges();
//
//     if (!cmakeSuccessful)
//     {
//         if (KMessageBox::questionYesNo( QApplication::activeWindow(),
//                                         i18n("Changes to CMakeLists failed, abort move?"),
//                                         DIALOG_CAPTION ) == KMessageBox::Yes)
//             return false;
//     }
//
//     QList<QUrl>::const_iterator it1=oldUrls.constBegin(), it1End=oldUrls.constEnd();
//     QList<QUrl>::const_iterator it2=movedUrls.constBegin();
//     Q_ASSERT(oldUrls.size()==movedUrls.size());
//     for(; it1!=it1End; ++it1, ++it2)
//     {
//         if (!KDevelop::renameUrl(project, *it1, *it2))
//             return false;
//
//         QList<ProjectBaseItem*> renamedItems = project->itemsForUrl(*it2);
//         bool dir = QFileInfo(it2->toLocalFile()).isDir();
//         foreach(ProjectBaseItem* item, renamedItems) {
//             if(dir)
//                 emit folderRenamed(Path(*it1), item->folder());
//             else
//                 emit fileRenamed(Path(*it1), item->file());
//         }
//     }
//
//     return true;
// }
//
// bool CMakeManager::copyFilesAndFolders(const KDevelop::Path::List &items, KDevelop::ProjectFolderItem* toFolder)
// {
//     IProject* project = toFolder->project();
//     foreach(const Path& path, items) {
//         if (!KDevelop::copyUrl(project, path.toUrl(), toFolder->url()))
//             return false;
//     }
//
//     return true;
// }
//
// bool CMakeManager::removeFilesAndFolders(const QList<KDevelop::ProjectBaseItem*> &items)
// {
//     using namespace CMakeEdit;
//
//     IProject* p = 0;
//     QList<QUrl> urls;
//     foreach(ProjectBaseItem* item, items)
//     {
//         Q_ASSERT(item->folder() || item->file());
//
//         urls += item->url();
//         if(!p)
//             p = item->project();
//     }
//
//     //First do CMakeLists changes
//     ApplyChangesWidget changesWidget;
//     changesWidget.setCaption(DIALOG_CAPTION);
//     changesWidget.setInformation(i18n("Remove files and folders from CMakeLists as follows:"));
//
//     bool cmakeSuccessful = changesWidgetRemoveItems(cmakeListedItemsAffectedByItemsChanged(items).toSet(), &changesWidget);
//
//     if (changesWidget.hasDocuments() && cmakeSuccessful)
//         cmakeSuccessful &= changesWidget.exec() && changesWidget.applyAllChanges();
//
//     if (!cmakeSuccessful)
//     {
//         if (KMessageBox::questionYesNo( QApplication::activeWindow(),
//                                         i18n("Changes to CMakeLists failed, abort deletion?"),
//                                         DIALOG_CAPTION ) == KMessageBox::Yes)
//             return false;
//     }
//
//     bool ret = true;
//     //Then delete the files/folders
//     foreach(const QUrl& file, urls)
//     {
//         ret &= KDevelop::removeUrl(p, file, QDir(file.toLocalFile()).exists());
//     }
//
//     return ret;
// }

bool CMakeManager::removeFilesFromTargets(const QList<ProjectFileItem*> &files)
{
//     using namespace CMakeEdit;
//
//     ApplyChangesWidget changesWidget;
//     changesWidget.setCaption(DIALOG_CAPTION);
//     changesWidget.setInformation(i18n("Modify project targets as follows:"));
//
//     if (!files.isEmpty() &&
//         changesWidgetRemoveFilesFromTargets(files, &changesWidget) &&
//         changesWidget.exec() &&
//         changesWidget.applyAllChanges()) {
//         return true;
//     }
    return false;
}

// ProjectFolderItem* CMakeManager::addFolder(const Path& folder, ProjectFolderItem* parent)
// {
//     using namespace CMakeEdit;
//
//     CMakeFolderItem *cmakeParent = nearestCMakeFolder(parent);
//     if(!cmakeParent)
//         return 0;
//
//     ApplyChangesWidget changesWidget;
//     changesWidget.setCaption(DIALOG_CAPTION);
//     changesWidget.setInformation(i18n("Create folder '%1':", folder.lastPathSegment()));
//
//     ///FIXME: use path in changes widget
//     changesWidgetAddFolder(folder.toUrl(), cmakeParent, &changesWidget);
//
//     if(changesWidget.exec() && changesWidget.applyAllChanges())
//     {
//         if(KDevelop::createFolder(folder.toUrl())) { //If saved we create the folder then the CMakeLists.txt file
//             Path newCMakeLists(folder, "CMakeLists.txt");
//             KDevelop::createFile( newCMakeLists.toUrl() );
//         } else
//             KMessageBox::error(0, i18n("Could not save the change."),
//                                   DIALOG_CAPTION);
//     }
//
//     return 0;
// }
//
// KDevelop::ProjectFileItem* CMakeManager::addFile( const Path& file, KDevelop::ProjectFolderItem* parent)
// {
//     KDevelop::ProjectFileItem* created = 0;
//     if ( KDevelop::createFile(file.toUrl()) ) {
//         QList< ProjectFileItem* > files = parent->project()->filesForPath(IndexedString(file.pathOrUrl()));
//         if(!files.isEmpty())
//             created = files.first();
//         else
//             created = new KDevelop::ProjectFileItem( parent->project(), file, parent );
//     }
//     return created;
// }

bool CMakeManager::addFilesToTarget(const QList< ProjectFileItem* > &_files, ProjectTargetItem* target)
{
    return false;
//     using namespace CMakeEdit;
//
//     const QSet<QString> headerExt = QSet<QString>() << ".h" << ".hpp" << ".hxx";
//     QList< ProjectFileItem* > files = _files;
//     for (int i = files.count() - 1; i >= 0; --i)
//     {
//         QString fileName = files[i]->fileName();
//         QString fileExt = fileName.mid(fileName.lastIndexOf('.'));
//         QList<ProjectBaseItem*> sameUrlItems = files[i]->project()->itemsForUrl(files[i]->url());
//         if (headerExt.contains(fileExt))
//             files.removeAt(i);
//         else foreach(ProjectBaseItem* item, sameUrlItems)
//         {
//             if (item->parent() == target)
//             {
//                 files.removeAt(i);
//                 break;
//             }
//         }
//     }
//
//     if(files.isEmpty())
//         return true;
//
//     ApplyChangesWidget changesWidget;
//     changesWidget.setCaption(DIALOG_CAPTION);
//     changesWidget.setInformation(i18n("Modify target '%1' as follows:", target->baseName()));
//
//     bool success = changesWidgetAddFilesToTarget(files, target, &changesWidget) &&
//                    changesWidget.exec() &&
//                    changesWidget.applyAllChanges();
//
//     if(!success)
//         KMessageBox::error(0, i18n("CMakeLists changes failed."), DIALOG_CAPTION);
//
//     return success;
}

// bool CMakeManager::renameFileOrFolder(ProjectBaseItem *item, const Path &newPath)
// {
//     using namespace CMakeEdit;
//
//     ApplyChangesWidget changesWidget;
//     changesWidget.setCaption(DIALOG_CAPTION);
//     changesWidget.setInformation(i18n("Rename '%1' to '%2':", item->text(),
//                                       newPath.lastPathSegment()));
//
//     bool cmakeSuccessful = true, changedCMakeLists=false;
//     IProject* project=item->project();
//     const Path oldPath=item->path();
//     QUrl oldUrl=oldPath.toUrl();
//     if (item->file())
//     {
//         QList<ProjectBaseItem*> targetFiles = cmakeListedItemsAffectedByUrlChange(project, oldUrl);
//         foreach(ProjectBaseItem* targetFile, targetFiles)
//             ///FIXME: use path in changes widget
//             cmakeSuccessful &= changesWidgetMoveTargetFile(targetFile, newPath.toUrl(), &changesWidget);
//     }
//     else if (CMakeFolderItem *folder = dynamic_cast<CMakeFolderItem*>(item))
//         ///FIXME: use path in changes widget
//         cmakeSuccessful &= changesWidgetRenameFolder(folder, newPath.toUrl(), &changesWidget);
//
//     item->setPath(newPath);
//     if (changesWidget.hasDocuments() && cmakeSuccessful) {
//         changedCMakeLists = changesWidget.exec() && changesWidget.applyAllChanges();
//         cmakeSuccessful &= changedCMakeLists;
//     }
//
//     if (!cmakeSuccessful)
//     {
//         if (KMessageBox::questionYesNo( QApplication::activeWindow(),
//             i18n("Changes to CMakeLists failed, abort rename?"),
//                                         DIALOG_CAPTION ) == KMessageBox::Yes)
//             return false;
//     }
//
//     bool ret = KDevelop::renameUrl(project, oldUrl, newPath.toUrl());
//     if(!ret) {
//         item->setPath(oldPath);
//     }
//     return ret;
// }
//
// bool CMakeManager::renameFile(ProjectFileItem *item, const Path &newPath)
// {
//     return renameFileOrFolder(item, newPath);
// }
//
// bool CMakeManager::renameFolder(ProjectFolderItem* item, const Path &newPath)
// {
//     return renameFileOrFolder(item, newPath);
// }

QWidget* CMakeManager::specialLanguageObjectNavigationWidget(const QUrl &url, const KTextEditor::Cursor& position)
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
        KTextEditor::Cursor start=position, end=position, step(0,1);
        for(QChar i=e->characterAt(start); i.isLetter() || i=='_'; i=e->characterAt(start-=step))
        {}
        start+=step;

        for(QChar i=e->characterAt(end); i.isLetter() || i=='_'; i=e->characterAt(end+=step))
        {}

        QString id=e->text(KTextEditor::Range(start, end));
        ICMakeDocumentation* docu=CMake::cmakeDocumentation();
        if( docu )
        {
            IDocumentation::Ptr desc=docu->description(id, url);
            if(desc)
            {
                doc=new CMakeNavigationWidget(top, desc);
            }
        }
    }

    return doc;
}

QPair<QString, QString> CMakeManager::cacheValue(KDevelop::IProject* project, const QString& id) const
{ return QPair<QString, QString>(); }
// {
//     QPair<QString, QString> ret;
//     if(project==0 && !m_projectsData.isEmpty())
//     {
//         project=m_projectsData.keys().first();
//     }
//
// //     qCDebug(CMAKE) << "cache value " << id << project << (m_projectsData.contains(project) && m_projectsData[project].cache.contains(id));
//     CMakeProjectData* data = m_projectsData[project];
//     if(data && data->cache.contains(id))
//     {
//         const CacheEntry& e=data->cache.value(id);
//         ret.first=e.value;
//         ret.second=e.doc;
//     }
//     return ret;
// }Add
//
void CMakeManager::projectClosing(IProject* p)
{
    m_projects.remove(p);
//     delete m_projectsData.take(p);
//     delete m_watchers.take(p);
//
//     m_filter->remove(p);
//
//     qCDebug(CMAKE) << "Project closed" << p;
}
//
// QStringList CMakeManager::processGeneratorExpression(const QStringList& expr, IProject* project, ProjectTargetItem* target) const
// {
//     QStringList ret;
//     const CMakeProjectData* data = m_projectsData[project];
//     GenerationExpressionSolver exec(data->properties, data->targetAlias);
//     if(target)
//         exec.setTargetName(target->text());
//
//     exec.defineVariable("INSTALL_PREFIX", data->vm.value("CMAKE_INSTALL_PREFIX").join(QString()));
//     for(QStringList::const_iterator it = expr.constBegin(), itEnd = expr.constEnd(); it!=itEnd; ++it) {
//         QStringList val = exec.run(*it).split(';');
//         ret += val;
//     }
//     return ret;
// }
/*
void CMakeManager::addPending(const Path& path, CMakeFolderItem* folder)
{
    m_pending.insert(path, folder);
}

CMakeFolderItem* CMakeManager::takePending(const Path& path)
{
    return m_pending.take(path);
}

void CMakeManager::addWatcher(IProject* p, const QString& path)
{
    if (QFileSystemWatcher* watcher = m_watchers.value(p)) {
        watcher->addPath(path);
    } else {
        qWarning() << "Could not find a watcher for project" << p << p->name() << ", path " << path;
        Q_ASSERT(false);
    }
}*/

// CMakeProjectData CMakeManager::projectData(IProject* project)
// {
//     Q_ASSERT(QThread::currentThread() == project->thread());
//     CMakeProjectData* data = m_projectsData[project];
//     if(!data) {
//         data = new CMakeProjectData;
//         m_projectsData[project] = data;
//     }
//     return *data;
// }

ProjectFilterManager* CMakeManager::filterManager() const
{
    return m_filter;
}

void CMakeManager::dirtyFile(const QString& path)
{
    qCDebug(CMAKE) << "dirty!" << path;

    //we initialize again hte project that sent the signal
    for(QHash<IProject*, CMakeProjectData>::const_iterator it = m_projects.constBegin(), itEnd = m_projects.constEnd(); it!=itEnd; ++it) {
        if(it->watcher == sender()) {
            import(it.key());
            break;
        }
    }
}

ProjectFolderItem* CMakeManager::createFolderItem(IProject* project, const Path& path, ProjectBaseItem* parent)
{
//     TODO: when we have data about targets, use folders with targets or similar
    if (QFile::exists(path.toLocalFile()+"/CMakeLists.txt"))
        return new KDevelop::ProjectBuildFolderItem( project, path, parent );
    else
        return KDevelop::AbstractFileManagerPlugin::createFolderItem(project, path, parent);
}

#include "cmakemanager.moc"
