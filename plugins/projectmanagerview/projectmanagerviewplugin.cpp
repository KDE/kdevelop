/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2016, 2017 Alexander Potashev <aspotashev@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "projectmanagerviewplugin.h"

#include <QApplication>
#include <QAction>
#include <QClipboard>
#include <QInputDialog>
#include <QList>
#include <QMimeData>
#include <QUrl>

#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageBox_KDevCompat>
#include <KParts/MainWindow>
#include <KPluginFactory>
#include <KIO/Paste>
#include <KFileItem>
#include <KUrlMimeData>

#include <project/projectmodel.h>
#include <project/projectbuildsetmodel.h>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <util/jobstatus.h>
#include <util/path.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iselectioncontroller.h>
#include <sublime/message.h>
#include <serialization/indexedstring.h>

#include "projectmanagerview.h"
#include "debug.h"
#include "cutcopypastehelpers.h"

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(ProjectManagerFactory, "kdevprojectmanagerview.json", registerPlugin<ProjectManagerViewPlugin>();)

namespace {

QAction* createSeparatorAction()
{
    auto* separator = new QAction(nullptr);
    separator->setSeparator(true);
    return separator;
}

// Returns nullptr iff the list of URLs to copy/cut was empty
QMimeData* createClipboardMimeData(const bool cut)
{
    auto* ctx = dynamic_cast<KDevelop::ProjectItemContext*>(
        ICore::self()->selectionController()->currentSelection());
    QList<QUrl> urls;
    QList<QUrl> mostLocalUrls;
    const auto& items = ctx->items();
    for (const ProjectBaseItem* item : items) {
        if (item->folder() || item->file()) {
            const QUrl& url = item->path().toUrl();
            urls << url;
            mostLocalUrls << KFileItem(url).mostLocalUrl();
        }
    }
    qCDebug(PLUGIN_PROJECTMANAGERVIEW) << urls;

    if (urls.isEmpty()) {
        return nullptr;
    }

    auto* mimeData = new QMimeData;
    KIO::setClipboardDataCut(mimeData, cut);
    KUrlMimeData::setUrls(urls, mostLocalUrls, mimeData);
    return mimeData;
}

} // anonymous namespace

class KDevProjectManagerViewFactory: public KDevelop::IToolViewFactory
{
    public:
        explicit KDevProjectManagerViewFactory( ProjectManagerViewPlugin *plugin ): mplugin( plugin )
        {}
        QWidget* create( QWidget *parent = nullptr ) override
        {
            return new ProjectManagerView( mplugin, parent );
        }
        Qt::DockWidgetArea defaultPosition() const override
        {
            return Qt::LeftDockWidgetArea;
        }
        QString id() const override
        {
            return QStringLiteral("org.kdevelop.ProjectsView");
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
    items.reserve(indexes.size());
    for (const auto& index : indexes) {
        items += model->itemFromIndex(index);
    }
    return items;
}

ProjectManagerViewPlugin::ProjectManagerViewPlugin( QObject *parent, const QVariantList& )
        : IPlugin( QStringLiteral("kdevprojectmanagerview"), parent ), d(new ProjectManagerViewPluginPrivate)
{
    d->m_buildAll = new QAction(i18nc("@action", "Build All Projects"), this);
    d->m_buildAll->setIcon(QIcon::fromTheme(QStringLiteral("run-build")));
    connect( d->m_buildAll, &QAction::triggered, this, &ProjectManagerViewPlugin::buildAllProjects );
    actionCollection()->addAction( QStringLiteral("project_buildall"), d->m_buildAll );

    d->m_build = new QAction(i18nc("@action", "Build Selection"), this);
    d->m_build->setIconText(i18nc("@action:intoolbar", "Build"));
    actionCollection()->setDefaultShortcut( d->m_build, Qt::Key_F8 );
    d->m_build->setIcon(QIcon::fromTheme(QStringLiteral("run-build")));
    d->m_build->setEnabled( false );
    connect( d->m_build, &QAction::triggered, this, &ProjectManagerViewPlugin::buildProjectItems );
    actionCollection()->addAction( QStringLiteral("project_build"), d->m_build );
    d->m_install = new QAction(i18nc("@action", "Install Selection"), this);
    d->m_install->setIconText(i18nc("@action:intoolbar", "Install"));
    d->m_install->setIcon(QIcon::fromTheme(QStringLiteral("run-build-install")));
    actionCollection()->setDefaultShortcut(d->m_install, Qt::SHIFT | Qt::Key_F8);
    d->m_install->setEnabled( false );
    connect( d->m_install, &QAction::triggered, this, &ProjectManagerViewPlugin::installProjectItems );
    actionCollection()->addAction( QStringLiteral("project_install"), d->m_install );
    d->m_clean = new QAction(i18nc("@action", "Clean Selection"), this);
    d->m_clean->setIconText(i18nc("@action:intoolbar", "Clean"));
    d->m_clean->setIcon(QIcon::fromTheme(QStringLiteral("run-build-clean")));
    d->m_clean->setEnabled( false );
    connect( d->m_clean, &QAction::triggered, this, &ProjectManagerViewPlugin::cleanProjectItems );
    actionCollection()->addAction( QStringLiteral("project_clean"), d->m_clean );
    d->m_configure = new QAction(i18nc("@action", "Configure Selection"), this);
    d->m_configure->setMenuRole( QAction::NoRole ); // OSX: Be explicit about role, prevent hiding due to conflict with "Preferences..." menu item 
    d->m_configure->setIconText(i18nc("@action:intoolbar", "Configure"));
    d->m_configure->setIcon(QIcon::fromTheme(QStringLiteral("run-build-configure")));
    d->m_configure->setEnabled( false );
    connect( d->m_configure, &QAction::triggered, this, &ProjectManagerViewPlugin::configureProjectItems );
    actionCollection()->addAction( QStringLiteral("project_configure"), d->m_configure );
    d->m_prune = new QAction(i18nc("@action", "Prune Selection"), this);
    d->m_prune->setIconText(i18nc("@action:intoolbar", "Prune"));
    d->m_prune->setIcon(QIcon::fromTheme(QStringLiteral("run-build-prune")));
    d->m_prune->setEnabled( false );
    connect( d->m_prune, &QAction::triggered, this, &ProjectManagerViewPlugin::pruneProjectItems );
    actionCollection()->addAction( QStringLiteral("project_prune"), d->m_prune );

    // only add the action so that its known in the actionCollection
    // and so that it's shortcut etc. pp. is restored
    // apparently that is not possible to be done in the view itself *sigh*
    auto locateDocumentAction = actionCollection()->addAction(QStringLiteral("locate_document"));
    locateDocumentAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionCollection()->setDefaultShortcut(locateDocumentAction, Qt::CTRL | Qt::Key_Less);

    setXMLFile( QStringLiteral("kdevprojectmanagerview.rc") );
    d->factory = new KDevProjectManagerViewFactory( this );
    core()->uiController()->addToolView(i18nc("@title:window", "Projects"), d->factory);
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
        isEmpty = !ctx || ctx->type() != Context::ProjectItemContext || static_cast<ProjectItemContext*>(ctx)->items().isEmpty();
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

ContextMenuExtension ProjectManagerViewPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    if( context->type() != KDevelop::Context::ProjectItemContext )
        return IPlugin::contextMenuExtension(context, parent);

    auto* ctx = static_cast<KDevelop::ProjectItemContext*>(context);
    const QList<KDevelop::ProjectBaseItem*> items = ctx->items();

    d->ctxProjectItemList.clear();

    if( items.isEmpty() )
        return IPlugin::contextMenuExtension(context, parent);

    //TODO: also needs: removeTarget, removeFileFromTarget, runTargetsFromContextMenu
    ContextMenuExtension menuExt;
    bool needsCreateFile = true;
    bool needsCreateFolder = true;
    bool needsCloseProjects = true;
    bool needsBuildItems = true;
    bool needsFolderItems = true;
    bool needsCutRenameRemove = true;
    bool needsRemoveTargetFiles = true;
    bool needsPaste = true;

    //needsCreateFile if there is one item and it's a folder or target
    needsCreateFile &= (items.count() == 1) && (items.first()->folder() || items.first()->target());
    //needsCreateFolder if there is one item and it's a folder
    needsCreateFolder &= (items.count() == 1) && (items.first()->folder());
    needsPaste = needsCreateFolder;

    d->ctxProjectItemList.reserve(items.size());
    for (ProjectBaseItem* item : items) {
        d->ctxProjectItemList << item->index();
        //needsBuildItems if items are limited to targets and buildfolders
        needsBuildItems &= item->target() || item->type() == ProjectBaseItem::BuildFolder;

        //needsCloseProjects if items are limited to top level folders (Project Folders)
        needsCloseProjects &= item->folder() && !item->folder()->parent();

        //needsFolderItems if items are limited to folders
        needsFolderItems &= (bool)item->folder();

        //needsRemove if items are limited to non-top-level folders or files that don't belong to targets
        needsCutRenameRemove &= (item->folder() && item->parent()) || (item->file() && !item->parent()->target());

        //needsRemoveTargets if items are limited to file items with target parents
        needsRemoveTargetFiles &= (item->file() && item->parent()->target());
    }

    if ( needsCreateFile ) {
        auto* action = new QAction(i18nc("@action:inmenu", "Create &File..."), parent);
        action->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::createFileFromContextMenu );
        menuExt.addAction( ContextMenuExtension::FileGroup, action );
    }
    if ( needsCreateFolder ) {
        auto* action = new QAction(i18nc("@action:inmenu", "Create F&older..."), parent);
        action->setIcon(QIcon::fromTheme(QStringLiteral("folder-new")));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::createFolderFromContextMenu );
        menuExt.addAction( ContextMenuExtension::FileGroup, action );
    }

    if ( needsBuildItems ) {
        auto* action = new QAction(i18nc("@action:inmenu", "&Build"), parent);
        action->setIcon(QIcon::fromTheme(QStringLiteral("run-build")));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::buildItemsFromContextMenu );
        menuExt.addAction( ContextMenuExtension::BuildGroup, action );
        action = new QAction(i18nc("@action:inmenu", "&Install"), parent);
        action->setIcon(QIcon::fromTheme(QStringLiteral("run-build-install")));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::installItemsFromContextMenu );
        menuExt.addAction( ContextMenuExtension::BuildGroup, action );
        action = new QAction(i18nc("@action:inmenu", "&Clean"), parent);
        action->setIcon(QIcon::fromTheme(QStringLiteral("run-build-clean")));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::cleanItemsFromContextMenu );
        menuExt.addAction( ContextMenuExtension::BuildGroup, action );
        action = new QAction(i18nc("@action:inmenu", "&Add to Build Set"), parent);
        action->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::addItemsFromContextMenuToBuildset );
        menuExt.addAction( ContextMenuExtension::BuildGroup, action );
    }

    if ( needsCloseProjects ) {
        auto* close = new QAction(i18ncp("@action:inmenu", "C&lose Project", "Close Projects", items.count()), parent);
        close->setIcon(QIcon::fromTheme(QStringLiteral("project-development-close")));
        connect( close, &QAction::triggered, this, &ProjectManagerViewPlugin::closeProjects );
        menuExt.addAction( ContextMenuExtension::ProjectGroup, close );
    }
    if ( needsFolderItems ) {
        auto* action = new QAction(i18nc("@action:inmenu", "&Reload"), parent);
        action->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
        connect( action, &QAction::triggered, this, &ProjectManagerViewPlugin::reloadFromContextMenu );
        menuExt.addAction( ContextMenuExtension::FileGroup, action );
    }

    // Populating cut/copy/paste group
    if ( !menuExt.actions(ContextMenuExtension::FileGroup).isEmpty() ) {
        menuExt.addAction( ContextMenuExtension::FileGroup, createSeparatorAction() );
    }
    if ( needsCutRenameRemove ) {
        QAction* cut = KStandardAction::cut(this, SLOT(cutFromContextMenu()), this);
        cut->setShortcutContext(Qt::WidgetShortcut);
        menuExt.addAction(ContextMenuExtension::FileGroup, cut);
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

    // Populating rename/remove group
    {
        menuExt.addAction( ContextMenuExtension::FileGroup, createSeparatorAction() );
    }
    if ( needsCutRenameRemove ) {
        auto* remove = new QAction(i18nc("@action:inmenu", "Remo&ve"), parent);
        remove->setIcon(QIcon::fromTheme(QStringLiteral("user-trash")));
        connect( remove, &QAction::triggered, this, &ProjectManagerViewPlugin::removeFromContextMenu );
        menuExt.addAction( ContextMenuExtension::FileGroup, remove );
        auto* rename = new QAction(i18nc("@action:inmenu", "Re&name..."), parent);
        rename->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));
        connect( rename, &QAction::triggered, this, &ProjectManagerViewPlugin::renameItemFromContextMenu );
        menuExt.addAction( ContextMenuExtension::FileGroup, rename );
    }
    if ( needsRemoveTargetFiles ) {
        auto* remove = new QAction(i18nc("@action:inmenu", "Remove from &Target"), parent);
        remove->setIcon(QIcon::fromTheme(QStringLiteral("user-trash")));
        connect( remove, &QAction::triggered, this, &ProjectManagerViewPlugin::removeTargetFilesFromContextMenu );
        menuExt.addAction( ContextMenuExtension::FileGroup, remove );
    }

    if ( needsCutRenameRemove || needsRemoveTargetFiles ) {
        menuExt.addAction(ContextMenuExtension::FileGroup, createSeparatorAction());
    }

    return menuExt;
}

void ProjectManagerViewPlugin::closeProjects()
{
    QList<KDevelop::IProject*> projectsToClose;
    ProjectModel* model = ICore::self()->projectController()->projectModel();
    for (const auto& index : std::as_const(d->ctxProjectItemList)) {
        KDevelop::ProjectBaseItem* item = model->itemFromIndex(index);
        if( !projectsToClose.contains( item->project() ) )
        {
            projectsToClose << item->project();
        }
    }
    d->ctxProjectItemList.clear();
    for (KDevelop::IProject* proj : std::as_const(projectsToClose)) {
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
    const auto projects = core()->projectController()->projects();
    items.reserve(projects.size());
    for (auto* project : projects) {
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
    const QList<BuildItem> buildItems = ICore::self()->projectController()->buildSetModel()->items();
    if( !buildItems.isEmpty() )
    {
        for (const BuildItem& buildItem : buildItems) {
            if( ProjectBaseItem* item = buildItem.findItem() )
            {
                items << item;
            }
        }

    } else
    {
        auto* ctx = static_cast<KDevelop::ProjectItemContext*>(ICore::self()->selectionController()->currentSelection());
        items = ctx->items();
    }

    return items;
}

void ProjectManagerViewPlugin::runBuilderJob( BuilderJob::BuildType type, const QList<ProjectBaseItem*>& items )
{
    auto* builder = new BuilderJob;
    builder->addItems( type, items );
    builder->updateJobName();
    ICore::self()->uiController()->registerStatus(new JobStatus(builder));
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
    const auto items = itemsFromIndexes(d->ctxProjectItemList);
    for (KDevelop::ProjectBaseItem* item : items) {
        ICore::self()->projectController()->buildSetModel()->addProjectItem( item );
    }
}

void ProjectManagerViewPlugin::runTargetsFromContextMenu( )
{
    const auto items = itemsFromIndexes(d->ctxProjectItemList);
    for (KDevelop::ProjectBaseItem* item : items) {
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
    const auto items = itemsFromIndexes(d->ctxProjectItemList);
    for (KDevelop::ProjectBaseItem* item : items) {
        if ( item->folder() ) {
            // since reloading should be recursive, only pass the upper-most items
            bool found = false;
            const auto currentFolders = folders;
            for (KDevelop::ProjectFolderItem* existing : currentFolders) {
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
    for (KDevelop::ProjectFolderItem* folder : std::as_const(folders)) {
        folder->project()->projectFileManager()->reload(folder);
    }
}

void ProjectManagerViewPlugin::createFolderFromContextMenu( )
{
    const auto items = itemsFromIndexes(d->ctxProjectItemList);
    for (KDevelop::ProjectBaseItem* item : items) {
        if ( item->folder() ) {
            QWidget* window(ICore::self()->uiController()->activeMainWindow()->window());
            QString name = QInputDialog::getText ( window,
                i18nc("@title:window", "Create Folder in %1", item->folder()->path().pathOrUrl() ),
                i18nc("@label:textbox", "Folder name:")
            );
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
    QHash< IProjectFileManager*, QList<KDevelop::ProjectBaseItem*> > filteredItems;
    QStringList itemPaths;
    for (KDevelop::ProjectBaseItem* item : std::as_const(sortedItems)) {
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

    if (KMessageBox::warningTwoActionsList(QApplication::activeWindow(),
                                           i18np("Do you really want to delete this item?",
                                                 "Do you really want to delete these %1 items?", itemPaths.size()),
                                           itemPaths, i18nc("@title:window", "Delete Files"), KStandardGuiItem::del(),
                                           KStandardGuiItem::cancel())
        == KMessageBox::SecondaryAction) {
        return;
    }

    //Go though projectmanagers, have them remove the files and folders that they own
    QHash< IProjectFileManager*, QList<KDevelop::ProjectBaseItem*> >::iterator it;
    for (it = filteredItems.begin(); it != filteredItems.end(); ++it)
    {
        Q_ASSERT(it.key());
        it.key()->removeFilesAndFolders(it.value());
    }
}

void ProjectManagerViewPlugin::removeTargetFilesFromContextMenu()
{
    const QList<ProjectBaseItem*> items = itemsFromIndexes( d->ctxProjectItemList );
    QHash< IBuildSystemManager*, QList<KDevelop::ProjectFileItem*> > itemsByBuildSystem;
    for (ProjectBaseItem* item : items) {
        itemsByBuildSystem[item->project()->buildSystemManager()].append(item->file());
    }

    QHash< IBuildSystemManager*, QList<KDevelop::ProjectFileItem*> >::iterator it;
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

    for (KDevelop::ProjectBaseItem* item : items) {
        if ((item->type()!=ProjectBaseItem::BuildFolder
                && item->type()!=ProjectBaseItem::Folder
                && item->type()!=ProjectBaseItem::File) || !item->parent())
        {
            continue;
        }

        const QString src = item->text();

        //Change QInputDialog->KFileSaveDialog?
        QString name = QInputDialog::getText(
            window, i18nc("@window:title", "Rename"),
            i18nc("@label:textbox", "New name for '%1':", item->text()),
            QLineEdit::Normal, item->text()
        );

        if (!name.isEmpty() && name != src) {
            ProjectBaseItem::RenameStatus status = item->rename( name );

            QString errorMessageText;
            switch(status) {
                case ProjectBaseItem::RenameOk:
                    break;
                case ProjectBaseItem::ExistingItemSameName:
                    errorMessageText = i18n("There is already a file named '%1'", name);
                    break;
                case ProjectBaseItem::ProjectManagerRenameFailed:
                    errorMessageText = i18n("Could not rename '%1'", name);
                    break;
                case ProjectBaseItem::InvalidNewName:
                    errorMessageText = i18n("'%1' is not a valid file name", name);
                    break;
            }
            if (!errorMessageText.isEmpty()) {
                auto* message = new Sublime::Message(errorMessageText, Sublime::Message::Error);
                ICore::self()->uiController()->postMessage(message);
            }
        }
    }
}

ProjectFileItem* createFile(const ProjectFolderItem* item)
{
    QWidget* window = ICore::self()->uiController()->activeMainWindow()->window();
    QString name = QInputDialog::getText(window, i18nc("@title:window", "Create File in %1", item->path().pathOrUrl()), i18nc("@label:textbox", "File name:"));

    if(name.isEmpty())
        return nullptr;

    ProjectFileItem* ret = item->project()->projectFileManager()->addFile( Path(item->path(), name), item->folder() );
    if (ret) {
        ICore::self()->documentController()->openDocument( ret->path().toUrl() );
    }
    return ret;
}

void ProjectManagerViewPlugin::createFileFromContextMenu( )
{
    const auto items = itemsFromIndexes(d->ctxProjectItemList);
    for (KDevelop::ProjectBaseItem* item : items) {
        if ( item->folder() ) {
            createFile(item->folder());
        } else if ( item->target() ) {
            auto* folder=dynamic_cast<ProjectFolderItem*>(item->parent());
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
    qApp->clipboard()->setMimeData(createClipboardMimeData(false));
}

void ProjectManagerViewPlugin::cutFromContextMenu()
{
    qApp->clipboard()->setMimeData(createClipboardMimeData(true));
}

static void selectItemsByPaths(ProjectManagerView* view, const Path::List& paths)
{
    KDevelop::ProjectModel* projectModel = KDevelop::ICore::self()->projectController()->projectModel();

    QList<ProjectBaseItem*> newItems;
    for (const Path& path : paths) {
        QList<ProjectBaseItem*> items = projectModel->itemsForPath(IndexedString(path.path()));
        newItems.append(items);
        for (ProjectBaseItem* item : std::as_const(items)) {
            view->expandItem(item->parent());
        }
    }
    view->selectItems(newItems);
}

void ProjectManagerViewPlugin::pasteFromContextMenu()
{
    auto* ctx = static_cast<KDevelop::ProjectItemContext*>(ICore::self()->selectionController()->currentSelection());
    if (ctx->items().count() != 1) {
        return; //do nothing if multiple or none items are selected
    }

    ProjectBaseItem* destItem = ctx->items().at(0);
    if (!destItem->folder()) {
        return; //do nothing if the target is not a directory
    }

    const QMimeData* data = qApp->clipboard()->mimeData();
    qCDebug(PLUGIN_PROJECTMANAGERVIEW) << data->urls();
    Path::List origPaths = toPathList(data->urls());
    const bool isCut = KIO::isClipboardDataCut(data);

    const CutCopyPasteHelpers::SourceToDestinationMap map = CutCopyPasteHelpers::mapSourceToDestination(origPaths, destItem->folder()->path());

    const QVector<CutCopyPasteHelpers::TaskInfo> tasks = CutCopyPasteHelpers::copyMoveItems(
        map.filteredPaths, destItem,
        isCut ? CutCopyPasteHelpers::Operation::CUT : CutCopyPasteHelpers::Operation::COPY);

    // Select new items in the project manager view
    auto* itemCtx = dynamic_cast<ProjectManagerViewItemContext*>(ICore::self()->selectionController()->currentSelection());
    if (itemCtx) {
        Path::List finalPathsList;
        for (const auto& task : tasks) {
            if (task.m_status == CutCopyPasteHelpers::TaskStatus::SUCCESS && task.m_type != CutCopyPasteHelpers::TaskType::DELETION) {
                finalPathsList.reserve(finalPathsList.size() + task.m_src.size());
                for (const Path& src : task.m_src) {
                    finalPathsList.append(map.finalPaths[src]);
                }
            }
        }

        selectItemsByPaths(itemCtx->view(), finalPathsList);
    }

    // If there was a single failure, display a warning dialog.
    const bool anyFailed = std::any_of(tasks.begin(), tasks.end(),
                                       [](const CutCopyPasteHelpers::TaskInfo& task) {
                                           return task.m_status != CutCopyPasteHelpers::TaskStatus::SUCCESS;
                                       });
    if (anyFailed) {
        QWidget* window = ICore::self()->uiController()->activeMainWindow()->window();
        showWarningDialogForFailedPaste(window, tasks);
    }
}

#include "projectmanagerviewplugin.moc"
#include "moc_projectmanagerviewplugin.cpp"
