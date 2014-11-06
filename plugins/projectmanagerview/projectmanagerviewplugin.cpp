/* This file is part of KDevelop
   Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
   Copyright 2007 Andreas Pakulat <apaku@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "projectmanagerviewplugin.h"

#include <QtCore/QList>
#include <QMimeData>
#include <QInputDialog>
#include <QApplication>
#include <QClipboard>
#include <QUrl>

#include <QAction>
#include <kactioncollection.h>
#include <kaboutdata.h>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <kparts/mainwindow.h>
#include <kpluginfactory.h>

#include <project/projectmodel.h>
#include <project/projectbuildsetmodel.h>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <project/interfaces/iprojectbuilder.h>
#include <util/path.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iselectioncontroller.h>

#include "projectmanagerview.h"
#include "debug.h"

using namespace KDevelop;

Q_LOGGING_CATEGORY(PLUGIN_PROJECTMANAGERVIEW, "kdevplatform.plugins.projectmanagerview")
K_PLUGIN_FACTORY_WITH_JSON(ProjectManagerFactory, "kdevprojectmanagerview.json", registerPlugin<ProjectManagerViewPlugin>();)

class KDevProjectManagerViewFactory: public KDevelop::IToolViewFactory
{
    public:
        KDevProjectManagerViewFactory( ProjectManagerViewPlugin *plugin ): mplugin( plugin )
        {}
        virtual QWidget* create( QWidget *parent = 0 )
        {
            return new ProjectManagerView( mplugin, parent );
        }
        virtual Qt::DockWidgetArea defaultPosition()
        {
            return Qt::LeftDockWidgetArea;
        }
        virtual QString id() const
        {
            return "org.kdevelop.ProjectsView";
        }
    private:
        ProjectManagerViewPlugin *mplugin;
};

class ProjectManagerViewPluginPrivate
{
public:
    ProjectManagerViewPluginPrivate()
    {}
    KDevProjectManagerViewFactory *factory;
    QList<QPersistentModelIndex> ctxProjectItemList;
    QAction* m_buildAll;
    QAction* m_build;
    QAction* m_install;
    QAction* m_clean;
    QAction* m_configure;
    QAction* m_prune;
};

static QList<ProjectBaseItem*> itemsFromIndexes(const QList<QPersistentModelIndex>& indexes)
{
    QList<ProjectBaseItem*> items;
    ProjectModel* model = ICore::self()->projectController()->projectModel();
    foreach(const QModelIndex& index, indexes) {
        items += model->itemFromIndex(index);
    }
    return items;
}

ProjectManagerViewPlugin::ProjectManagerViewPlugin( QObject *parent, const QVariantList& )
        : IPlugin( "kdevprojectmanagerview", parent ), d(new ProjectManagerViewPluginPrivate)
{
    d->m_buildAll = new QAction( i18n("Build all Projects"), this );
    d->m_buildAll->setIcon(QIcon::fromTheme("run-build"));
    connect( d->m_buildAll, &QAction::triggered, this, &ProjectManagerViewPlugin::buildAllProjects );
    actionCollection()->addAction( "project_buildall", d->m_buildAll );

    d->m_build = new QAction( i18n("Build Selection"), this );
    d->m_build->setIconText( i18n("Build") );
    actionCollection()->setDefaultShortcut( d->m_build, Qt::Key_F8 );
    d->m_build->setIcon(QIcon::fromTheme("run-build"));
    d->m_build->setEnabled( false );
    connect( d->m_build, &QAction::triggered, this, &ProjectManagerViewPlugin::buildProjectItems );
    actionCollection()->addAction( "project_build", d->m_build );
    d->m_install = new QAction( i18n("Install Selection"), this );
    d->m_install->setIconText( i18n("Install") );
    d->m_install->setIcon(QIcon::fromTheme("run-build-install"));
    actionCollection()->setDefaultShortcut( d->m_install, Qt::SHIFT + Qt::Key_F8 );
    d->m_install->setEnabled( false );
    connect( d->m_install, &QAction::triggered, this, &ProjectManagerViewPlugin::installProjectItems );
    actionCollection()->addAction( "project_install", d->m_install );
    d->m_clean = new QAction( i18n("Clean Selection"), this );
    d->m_clean->setIconText( i18n("Clean") );
    d->m_clean->setIcon(QIcon::fromTheme("run-build-clean"));
    d->m_clean->setEnabled( false );
    connect( d->m_clean, &QAction::triggered, this, &ProjectManagerViewPlugin::cleanProjectItems );
    actionCollection()->addAction( "project_clean", d->m_clean );
    d->m_configure = new QAction( i18n("Configure Selection"), this );
    d->m_configure->setIconText( i18n("Configure") );
    d->m_configure->setIcon(QIcon::fromTheme("run-build-configure"));
    d->m_configure->setEnabled( false );
    connect( d->m_configure, &QAction::triggered, this, &ProjectManagerViewPlugin::configureProjectItems );
    actionCollection()->addAction( "project_configure", d->m_configure );
    d->m_prune = new QAction( i18n("Prune Selection"), this );
    d->m_prune->setIconText( i18n("Prune") );
    d->m_prune->setIcon(QIcon::fromTheme("run-build-prune"));
    d->m_prune->setEnabled( false );
    connect( d->m_prune, &QAction::triggered, this, &ProjectManagerViewPlugin::pruneProjectItems );
    actionCollection()->addAction( "project_prune", d->m_prune );
    // only add the action so that its known in the actionCollection
    // and so that it's shortcut etc. pp. is restored
    // apparently that is not possible to be done in the view itself *sigh*
    actionCollection()->addAction( "locate_document" );
    setXMLFile( "kdevprojectmanagerview.rc" );
    d->factory = new KDevProjectManagerViewFactory( this );
    core()->uiController()->addToolView( i18n("Projects"), d->factory );
    connect(core()->selectionController(), &ISelectionController::selectionChanged,
             this, &ProjectManagerViewPlugin::updateActionState);
    connect(ICore::self()->projectController()->buildSetModel(), &KDevelop::ProjectBuildSetModel::rowsInserted,
             this, &ProjectManagerViewPlugin::updateFromBuildSetChange);
    connect(ICore::self()->projectController()->buildSetModel(), &KDevelop::ProjectBuildSetModel::rowsRemoved,
             this, &ProjectManagerViewPlugin::updateFromBuildSetChange);
    connect(ICore::self()->projectController()->buildSetModel(), &KDevelop::ProjectBuildSetModel::modelReset,
             this, &ProjectManagerViewPlugin::updateFromBuildSetChange);
}

void ProjectManagerViewPlugin::updateFromBuildSetChange()
{
    updateActionState( core()->selectionController()->currentSelection() );
}

void ProjectManagerViewPlugin::updateActionState( KDevelop::Context* ctx )
{
    bool isEmpty = ICore::self()->projectController()->buildSetModel()->items().isEmpty();
    if( isEmpty )
    {
        isEmpty = !ctx || ctx->type() != Context::ProjectItemContext || dynamic_cast<ProjectItemContext*>( ctx )->items().isEmpty();
    }
    d->m_build->setEnabled( !isEmpty );
    d->m_install->setEnabled( !isEmpty );
    d->m_clean->setEnabled( !isEmpty );
    d->m_configure->setEnabled( !isEmpty );
    d->m_prune->setEnabled( !isEmpty );
}

ProjectManagerViewPlugin::~ProjectManagerViewPlugin()
{
    delete d;
}

void ProjectManagerViewPlugin::unload()
{
    qCDebug(PLUGIN_PROJECTMANAGERVIEW) << "unloading manager view";
    core()->uiController()->removeToolView(d->factory);
}

ContextMenuExtension ProjectManagerViewPlugin::contextMenuExtension( KDevelop::Context* context )
{
    if( context->type() != KDevelop::Context::ProjectItemContext )
        return IPlugin::contextMenuExtension( context );

    KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
    QList<KDevelop::ProjectBaseItem*> items = ctx->items();

    d->ctxProjectItemList.clear();

    if( items.isEmpty() )
        return IPlugin::contextMenuExtension( context );

    //TODO: also needs: removeTarget, removeFileFromTarget, runTargetsFromContextMenu
    ContextMenuExtension menuExt;
    bool needsCreateFile = true;
    bool needsCreateFolder = true;
    bool needsCloseProjects = true;
    bool needsBuildItems = true;
    bool needsFolderItems = true;
    bool needsRemoveAndRename = true;
    bool needsRemoveTargetFiles = true;
    bool needsPaste = true;

    //needsCreateFile if there is one item and it's a folder or target
    needsCreateFile &= (items.count() == 1) && (items.first()->folder() || items.first()->target());
    //needsCreateFolder if there is one item and it's a folder
    needsCreateFolder &= (items.count() == 1) && (items.first()->folder());
    needsPaste = needsCreateFolder;

    foreach( ProjectBaseItem* item, items ) {
        d->ctxProjectItemList << item->index();
        //needsBuildItems if items are limited to targets and buildfolders
        needsBuildItems &= item->target() || item->type() == ProjectBaseItem::BuildFolder;

        //needsCloseProjects if items are limited to top level folders (Project Folders)
        needsCloseProjects &= item->folder() && !item->folder()->parent();

        //needsFolderItems if items are limited to folders
        needsFolderItems &= (bool)item->folder();

        //needsRemove if items are limited to non-top-level folders or files that don't belong to targets
        needsRemoveAndRename &= (item->folder() && item->parent()) || (item->file() && !item->parent()->target());

        //needsRemoveTargets if items are limited to file items with target parents
        needsRemoveTargetFiles &= (item->file() && item->parent()->target());
    }

    if ( needsCreateFile ) {
        QAction* action = new QAction( i18n( "Create File" ), this );
        action->setIcon(QIcon::fromTheme("document-new"));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::createFileFromContextMenu );
        menuExt.addAction( ContextMenuExtension::FileGroup, action );
    }
    if ( needsCreateFolder ) {
        QAction* action = new QAction( i18n( "Create Folder" ), this );
        action->setIcon(QIcon::fromTheme("folder-new"));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::createFolderFromContextMenu );
        menuExt.addAction( ContextMenuExtension::FileGroup, action );
    }

    if ( needsBuildItems ) {
        QAction* action = new QAction( i18nc( "@action", "Build" ), this );
        action->setIcon(QIcon::fromTheme("run-build"));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::buildItemsFromContextMenu );
        menuExt.addAction( ContextMenuExtension::BuildGroup, action );
        action = new QAction( i18nc( "@action", "Install" ), this );
        action->setIcon(QIcon::fromTheme("run-install"));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::installItemsFromContextMenu );
        menuExt.addAction( ContextMenuExtension::BuildGroup, action );
        action = new QAction( i18nc( "@action", "Clean" ), this );
        action->setIcon(QIcon::fromTheme("run-clean"));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::cleanItemsFromContextMenu );
        menuExt.addAction( ContextMenuExtension::BuildGroup, action );
        action = new QAction( i18n( "Add to Build Set" ), this );
        action->setIcon(QIcon::fromTheme("list-add"));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::addItemsFromContextMenuToBuildset );
        menuExt.addAction( ContextMenuExtension::BuildGroup, action );
    }

    if ( needsCloseProjects ) {
        QAction* close = new QAction( i18np( "Close Project", "Close Projects", items.count() ), this );
        close->setIcon(QIcon::fromTheme("project-development-close"));
        connect( close, &QAction::triggered, this, &ProjectManagerViewPlugin::closeProjects );
        menuExt.addAction( ContextMenuExtension::ProjectGroup, close );
    }
    if ( needsFolderItems ) {
        QAction* action = new QAction( i18n( "Reload" ), this );
        action->setIcon(QIcon::fromTheme("view-refresh"));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::reloadFromContextMenu );
        menuExt.addAction( ContextMenuExtension::FileGroup, action );
    }
    if ( needsRemoveAndRename ) {
        QAction* remove = new QAction( i18n( "Remove" ), this );
        remove->setIcon(QIcon::fromTheme("user-trash"));
        connect( remove, &QAction::triggered, this, &ProjectManagerViewPlugin::removeFromContextMenu );
        menuExt.addAction( ContextMenuExtension::FileGroup, remove );
        QAction* rename = new QAction( i18n( "Rename" ), this );
        rename->setIcon(QIcon::fromTheme("edit-rename"));
        connect( rename, &QAction::triggered, this, &ProjectManagerViewPlugin::renameItemFromContextMenu );
        menuExt.addAction( ContextMenuExtension::FileGroup, rename );
    }
    if ( needsRemoveTargetFiles ) {
        QAction* remove = new QAction( i18n( "Remove From Target" ), this );
        remove->setIcon(QIcon::fromTheme("user-trash"));
        connect( remove, &QAction::triggered, this, &ProjectManagerViewPlugin::removeTargetFilesFromContextMenu );
        menuExt.addAction( ContextMenuExtension::FileGroup, remove );
    }

    {
        QAction* copy = KStandardAction::copy(this, SLOT(copyFromContextMenu()), this);
        copy->setShortcutContext(Qt::WidgetShortcut);
        menuExt.addAction( ContextMenuExtension::FileGroup, copy );
    }
    if (needsPaste) {
        QAction* paste = KStandardAction::paste(this, SLOT(pasteFromContextMenu()), this);
        paste->setShortcutContext(Qt::WidgetShortcut);
        menuExt.addAction( ContextMenuExtension::FileGroup, paste );
    }

    return menuExt;
}

void ProjectManagerViewPlugin::closeProjects()
{
    QList<KDevelop::IProject*> projectsToClose;
    ProjectModel* model = ICore::self()->projectController()->projectModel();
    foreach( const QModelIndex& index, d->ctxProjectItemList )
    {
        KDevelop::ProjectBaseItem* item = model->itemFromIndex(index);
        if( !projectsToClose.contains( item->project() ) )
        {
            projectsToClose << item->project();
        }
    }
    d->ctxProjectItemList.clear();
    foreach( KDevelop::IProject* proj, projectsToClose )
    {
        core()->projectController()->closeProject( proj );
    }
}


void ProjectManagerViewPlugin::installItemsFromContextMenu()
{
    runBuilderJob( BuilderJob::Install, itemsFromIndexes(d->ctxProjectItemList) );
    d->ctxProjectItemList.clear();
}

void ProjectManagerViewPlugin::cleanItemsFromContextMenu()
{
    runBuilderJob( BuilderJob::Clean, itemsFromIndexes( d->ctxProjectItemList ) );
    d->ctxProjectItemList.clear();
}

void ProjectManagerViewPlugin::buildItemsFromContextMenu()
{
    runBuilderJob( BuilderJob::Build, itemsFromIndexes( d->ctxProjectItemList ) );
    d->ctxProjectItemList.clear();
}

QList<ProjectBaseItem*> ProjectManagerViewPlugin::collectAllProjects()
{
    QList<KDevelop::ProjectBaseItem*> items;
    foreach( KDevelop::IProject* project, core()->projectController()->projects() )
    {
        items << project->projectItem();
    }
    return items;
}

void ProjectManagerViewPlugin::buildAllProjects()
{
    runBuilderJob( BuilderJob::Build, collectAllProjects() );
}

QList<ProjectBaseItem*> ProjectManagerViewPlugin::collectItems()
{
    QList<ProjectBaseItem*> items;
    QList<BuildItem> buildItems = ICore::self()->projectController()->buildSetModel()->items();
    if( !buildItems.isEmpty() )
    {
        foreach( const BuildItem& buildItem, buildItems )
        {
            if( ProjectBaseItem* item = buildItem.findItem() )
            {
                items << item;
            }
        }

    } else
    {
        KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>(ICore::self()->selectionController()->currentSelection());
        items = ctx->items();
    }

    return items;
}

void ProjectManagerViewPlugin::runBuilderJob( BuilderJob::BuildType type, QList<ProjectBaseItem*> items )
{
    BuilderJob* builder = new BuilderJob;
    builder->addItems( type, items );
    builder->updateJobName();
    ICore::self()->runController()->registerJob( builder );
}

void ProjectManagerViewPlugin::installProjectItems()
{
    runBuilderJob( KDevelop::BuilderJob::Install, collectItems() );
}

void ProjectManagerViewPlugin::pruneProjectItems()
{
    runBuilderJob( KDevelop::BuilderJob::Prune, collectItems() );
}

void ProjectManagerViewPlugin::configureProjectItems()
{
    runBuilderJob( KDevelop::BuilderJob::Configure, collectItems() );
}

void ProjectManagerViewPlugin::cleanProjectItems()
{
    runBuilderJob( KDevelop::BuilderJob::Clean, collectItems() );
}

void ProjectManagerViewPlugin::buildProjectItems()
{
    runBuilderJob( KDevelop::BuilderJob::Build, collectItems() );
}

void ProjectManagerViewPlugin::addItemsFromContextMenuToBuildset( )
{
    foreach( KDevelop::ProjectBaseItem* item, itemsFromIndexes( d->ctxProjectItemList ))
    {
        ICore::self()->projectController()->buildSetModel()->addProjectItem( item );
    }
}

void ProjectManagerViewPlugin::runTargetsFromContextMenu( )
{
    foreach( KDevelop::ProjectBaseItem* item, itemsFromIndexes( d->ctxProjectItemList ))
    {
        KDevelop::ProjectExecutableTargetItem* t=item->executable();
        if(t)
        {
            qCDebug(PLUGIN_PROJECTMANAGERVIEW) << "Running target: " << t->text() << t->builtUrl();
        }
    }
}

void ProjectManagerViewPlugin::projectConfiguration( )
{
    if( !d->ctxProjectItemList.isEmpty() )
    {
        ProjectModel* model = ICore::self()->projectController()->projectModel();
        core()->projectController()->configureProject( model->itemFromIndex(d->ctxProjectItemList.at( 0 ))->project() );
    }
}

void ProjectManagerViewPlugin::reloadFromContextMenu( )
{
    QList< KDevelop::ProjectFolderItem* > folders;
    foreach( KDevelop::ProjectBaseItem* item, itemsFromIndexes( d->ctxProjectItemList ) )
    {
        if ( item->folder() ) {
            // since reloading should be recursive, only pass the upper-most items
            bool found = false;
            foreach ( KDevelop::ProjectFolderItem* existing, folders ) {
                if ( existing->path().isParentOf(item->folder()->path()) ) {
                    // simply skip this child
                    found = true;
                    break;
                } else if ( item->folder()->path().isParentOf(existing->path()) ) {
                    // remove the child in the list and add the current item instead
                    folders.removeOne(existing);
                    // continue since there could be more than one existing child
                }
            }
            if ( !found ) {
                folders << item->folder();
            }
        }
    }
    foreach( KDevelop::ProjectFolderItem* folder, folders ) {
        folder->project()->projectFileManager()->reload(folder);
    }
}

void ProjectManagerViewPlugin::createFolderFromContextMenu( )
{
    foreach( KDevelop::ProjectBaseItem* item, itemsFromIndexes( d->ctxProjectItemList ))
    {
        if ( item->folder() ) {
            QWidget* window(ICore::self()->uiController()->activeMainWindow()->window());
            QString name = QInputDialog::getText ( window,
                                i18n ( "Create Folder in %1", item->folder()->path().pathOrUrl() ), i18n ( "Folder Name" ) );
            if (!name.isEmpty()) {
                item->project()->projectFileManager()->addFolder( Path(item->path(), name), item->folder() );
            }
        }
    }
}

void ProjectManagerViewPlugin::removeFromContextMenu()
{
    removeItems(itemsFromIndexes( d->ctxProjectItemList ));
}

void ProjectManagerViewPlugin::removeItems(const QList< ProjectBaseItem* >& items)
{
    if (items.isEmpty()) {
        return;
    }

    //copy the list of selected items and sort it to guarantee parents will come before children
    QList<KDevelop::ProjectBaseItem*> sortedItems = items;
    std::sort(sortedItems.begin(), sortedItems.end(), ProjectBaseItem::pathLessThan);

    Path lastFolder;
    QMap< IProjectFileManager*, QList<KDevelop::ProjectBaseItem*> > filteredItems;
    QStringList itemPaths;
    foreach( KDevelop::ProjectBaseItem* item, sortedItems )
    {
        if (item->isProjectRoot()) {
            continue;
        } else if (item->folder() || item->file()) {
            //make sure no children of folders that will be deleted are listed
            if (lastFolder.isParentOf(item->path())) {
                continue;
            } else if (item->folder()) {
                lastFolder = item->path();
            }

            IProjectFileManager* manager = item->project()->projectFileManager();
            if (manager) {
                filteredItems[manager] << item;
                itemPaths << item->path().pathOrUrl();
            }
        }
    }

    if (filteredItems.isEmpty()) {
        return;
    }

    if (KMessageBox::warningYesNoList(
            QApplication::activeWindow(),
            i18np("Do you really want to delete this item?",
                  "Do you really want to delete these %1 items?",
                  itemPaths.size()),
            itemPaths, i18n("Delete Files"),
            KStandardGuiItem::del(), KStandardGuiItem::cancel()
        ) == KMessageBox::No) {
        return;
    }

    //Go though projectmanagers, have them remove the files and folders that they own
    QMap< IProjectFileManager*, QList<KDevelop::ProjectBaseItem*> >::iterator it;
    for (it = filteredItems.begin(); it != filteredItems.end(); ++it)
    {
        Q_ASSERT(it.key());
        it.key()->removeFilesAndFolders(it.value());
    }
}

void ProjectManagerViewPlugin::removeTargetFilesFromContextMenu()
{
    QList<ProjectBaseItem*> items = itemsFromIndexes( d->ctxProjectItemList );
    QMap< IBuildSystemManager*, QList<KDevelop::ProjectFileItem*> > itemsByBuildSystem;
    foreach(ProjectBaseItem *item, items)
        itemsByBuildSystem[item->project()->buildSystemManager()].append(item->file());

    QMap< IBuildSystemManager*, QList<KDevelop::ProjectFileItem*> >::iterator it;
    for (it = itemsByBuildSystem.begin(); it != itemsByBuildSystem.end(); ++it)
        it.key()->removeFilesFromTargets(it.value());
}

void ProjectManagerViewPlugin::renameItemFromContextMenu()
{
    renameItems(itemsFromIndexes( d->ctxProjectItemList ));
}

void ProjectManagerViewPlugin::renameItems(const QList< ProjectBaseItem* >& items)
{
    if (items.isEmpty()) {
        return;
    }

    QWidget* window = ICore::self()->uiController()->activeMainWindow()->window();

    foreach( KDevelop::ProjectBaseItem* item, items )
    {
        if ((item->type()!=ProjectBaseItem::BuildFolder
                && item->type()!=ProjectBaseItem::Folder
                && item->type()!=ProjectBaseItem::File) || !item->parent())
        {
            continue;
        }

        const QString src = item->text();

        //Change QInputDialog->KFileSaveDialog?
        QString name = QInputDialog::getText(
            window, i18n("Rename..."),
            i18n("New name for '%1':", item->text()),
            QLineEdit::Normal, item->text()
        );

        if (!name.isEmpty() && name != src) {
            ProjectBaseItem::RenameStatus status = item->rename( name );

            switch(status) {
                case ProjectBaseItem::RenameOk:
                    break;
                case ProjectBaseItem::ExistingItemSameName:
                    KMessageBox::error(window, i18n("There is already a file named '%1'", name));
                    break;
                case ProjectBaseItem::ProjectManagerRenameFailed:
                    KMessageBox::error(window, i18n("Could not rename '%1'", name));
                    break;
                case ProjectBaseItem::InvalidNewName:
                    KMessageBox::error(window, i18n("'%1' is not a valid file name", name));
                    break;
            }
        }
    }
}

ProjectFileItem* createFile(const ProjectFolderItem* item)
{
    QWidget* window = ICore::self()->uiController()->activeMainWindow()->window();
    QString name = QInputDialog::getText(window, i18n("Create File in %1", item->path().pathOrUrl()), i18n("File name:"));

    if(name.isEmpty())
        return 0;

    ProjectFileItem* ret = item->project()->projectFileManager()->addFile( Path(item->path(), name), item->folder() );
    if (ret) {
        ICore::self()->documentController()->openDocument( ret->path().toUrl() );
    }
    return ret;
}

void ProjectManagerViewPlugin::createFileFromContextMenu( )
{
    foreach( KDevelop::ProjectBaseItem* item, itemsFromIndexes( d->ctxProjectItemList ) )
    {
        if ( item->folder() ) {
            createFile(item->folder());
        } else if ( item->target() ) {
            ProjectFolderItem* folder=dynamic_cast<ProjectFolderItem*>(item->parent());
            if(folder)
            {
                ProjectFileItem* f=createFile(folder);
                if(f)
                    item->project()->buildSystemManager()->addFilesToTarget(QList<ProjectFileItem*>() << f, item->target());
            }
        }
    }
}

void ProjectManagerViewPlugin::copyFromContextMenu()
{
    KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>(ICore::self()->selectionController()->currentSelection());
    QList<QUrl> urls;
    foreach (ProjectBaseItem* item, ctx->items()) {
        if (item->folder() || item->file()) {
            urls << item->path().toUrl();
        }
    }
    qCDebug(PLUGIN_PROJECTMANAGERVIEW) << urls;
    if (!urls.isEmpty()) {
        QMimeData* data = new QMimeData;
        data->setUrls(urls);
        qApp->clipboard()->setMimeData(data);
    }
}

void ProjectManagerViewPlugin::pasteFromContextMenu()
{
    KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>(ICore::self()->selectionController()->currentSelection());
    if (ctx->items().count() != 1)
        return; //do nothing if multiple or none items are selected

    ProjectBaseItem* destItem = ctx->items().first();
    if (!destItem->folder())
        return; //do nothing if the target is not a directory

    const QMimeData* data = qApp->clipboard()->mimeData();
    qCDebug(PLUGIN_PROJECTMANAGERVIEW) << data->urls();
    const Path::List paths = toPathList(data->urls());
    bool success = destItem->project()->projectFileManager()->copyFilesAndFolders(paths, destItem->folder());

    if (success) {
        ProjectManagerViewItemContext* viewCtx = dynamic_cast<ProjectManagerViewItemContext*>(ICore::self()->selectionController()->currentSelection());
        if (viewCtx) {

            //expand target folder
            viewCtx->view()->expandItem(destItem);

            //and select new items
            QList<ProjectBaseItem*> newItems;
            foreach (const Path &path, paths) {
                const Path targetPath(destItem->path(), path.lastPathSegment());
                foreach (ProjectBaseItem *item, destItem->children()) {
                    if (item->path() == targetPath) {
                        newItems << item;
                    }
                }
            }
            viewCtx->view()->selectItems(newItems);
        }
    }
}


#include "projectmanagerviewplugin.moc"

