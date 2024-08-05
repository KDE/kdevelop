/*
    SPDX-FileCopyrightText: 2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "projecttreeview.h"


#include <QAction>
#include <QAbstractProxyModel>
#include <QKeyEvent>
#include <QApplication>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>

#include <KConfigGroup>
#include <KLocalizedString>

#include <project/projectmodel.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/context.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iselectioncontroller.h>
#include <interfaces/isession.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include "projectmanagerviewplugin.h"
#include "projectmodelsaver.h"
#include "projectmodelitemdelegate.h"
#include "debug.h"
#include <project/projectutils.h>
#include <widgetcolorizer.h>

using namespace KDevelop;

namespace {

QString settingsConfigGroup() { return QStringLiteral("ProjectTreeView"); }

QList<ProjectFileItem*> fileItemsWithin(const QList<ProjectBaseItem*>& items)
{
    QList<ProjectFileItem*> fileItems;
    fileItems.reserve(items.size());
    for (ProjectBaseItem* item : items) {
        if (ProjectFileItem *file = item->file())
            fileItems.append(file);
        else if (item->folder())
            fileItems.append(fileItemsWithin(item->children()));
    }
    return fileItems;
}

QList<ProjectBaseItem*> topLevelItemsWithin(QList<ProjectBaseItem*> items)
{
    std::sort(items.begin(), items.end(), ProjectBaseItem::pathLessThan);
    Path lastFolder;
    for (int i = items.size() - 1; i >= 0; --i)
    {
        if (lastFolder.isParentOf(items[i]->path()))
            items.removeAt(i);
        else if (items[i]->folder())
            lastFolder = items[i]->path();
    }
    return items;
}

template<class T>
void filterDroppedItems(QList<T*> &items, ProjectBaseItem* dest)
{
    for (int i = items.size() - 1; i >= 0; --i)
    {
        //No drag and drop from and to same location
        if (items[i]->parent() == dest)
            items.removeAt(i);
        //No moving between projects (technically feasible if the projectmanager is the same though...)
        else if (items[i]->project() != dest->project())
            items.removeAt(i);
    }
}

//TODO test whether this could be replaced by projectbuildsetwidget.cpp::showContextMenu_appendActions
void popupContextMenu_appendActions(QMenu& menu, const QList<QAction*>& actions)
{
    menu.addActions(actions);
    menu.addSeparator();
}

}

ProjectTreeView::ProjectTreeView( QWidget *parent )
        : QTreeView( parent ), m_previousSelection ( nullptr )
{
    header()->hide();

    setEditTriggers( QAbstractItemView::EditKeyPressed );

    setContextMenuPolicy( Qt::CustomContextMenu );
    setSelectionMode( QAbstractItemView::ExtendedSelection );

    setIndentation(10);

    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setAutoScroll(true);
    setAutoExpandDelay(300);
    setItemDelegate(new ProjectModelItemDelegate(this));

    connect( this, &ProjectTreeView::customContextMenuRequested, this, &ProjectTreeView::popupContextMenu );
    connect( this, &ProjectTreeView::activated, this, &ProjectTreeView::slotActivated );

    connect( ICore::self(), &ICore::aboutToShutdown,
             this, &ProjectTreeView::aboutToShutdown);
    connect( ICore::self()->projectController(), &IProjectController::projectOpened,
             this, &ProjectTreeView::restoreState );
    connect( ICore::self()->projectController(), &IProjectController::projectClosed,
             this, &ProjectTreeView::projectClosed );
}

ProjectTreeView::~ProjectTreeView()
{
}

ProjectBaseItem* ProjectTreeView::itemAtPos(const QPoint& pos) const
{
    return indexAt(pos).data(ProjectModel::ProjectItemRole).value<ProjectBaseItem*>();
}

void ProjectTreeView::dropEvent(QDropEvent* event)
{
    auto* selectionCtxt =
            static_cast<ProjectItemContext*>(KDevelop::ICore::self()->selectionController()->currentSelection());
    ProjectBaseItem* destItem = itemAtPos(event->position().toPoint());
    if (destItem && (dropIndicatorPosition() == AboveItem || dropIndicatorPosition() == BelowItem))
            destItem = destItem->parent();
    if (selectionCtxt && destItem)
    {
        if (ProjectFolderItem *folder = destItem->folder())
        {
            QMenu dropMenu(this);

            QString seq = QKeySequence( Qt::ShiftModifier ).toString();
            seq.chop(1); // chop superfluous '+'
            auto* move = new QAction(i18nc("@action:inmenu", "&Move Here") + QLatin1Char('\t') + seq, &dropMenu);
            move->setIcon(QIcon::fromTheme(QStringLiteral("edit-move"), QIcon::fromTheme(QStringLiteral("go-jump"))));
            dropMenu.addAction(move);

            seq = QKeySequence( Qt::ControlModifier ).toString();
            seq.chop(1);
            auto* copy = new QAction(i18nc("@action:inmenu", "&Copy Here") + QLatin1Char('\t') + seq, &dropMenu);
            copy->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
            dropMenu.addAction(copy);

            dropMenu.addSeparator();

            auto* cancel = new QAction(i18nc("@action:inmenu", "C&ancel") + QLatin1Char('\t') + QKeySequence(Qt::Key_Escape).toString(), &dropMenu);
            cancel->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
            dropMenu.addAction(cancel);

            QAction *executedAction = nullptr;

            Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
            if (modifiers == Qt::ControlModifier) {
                executedAction = copy;
            } else if (modifiers == Qt::ShiftModifier) {
                executedAction = move;
            } else {
                executedAction = dropMenu.exec(this->mapToGlobal(event->position().toPoint()));
            }

            QList<ProjectBaseItem*> usefulItems = topLevelItemsWithin(selectionCtxt->items());
            filterDroppedItems(usefulItems, destItem);
            Path::List paths;
            paths.reserve(usefulItems.size());
            for (ProjectBaseItem* i : std::as_const(usefulItems)) {
                paths << i->path();
            }
            bool success = false;
            if (executedAction == copy) {
                success = destItem->project()->projectFileManager()->copyFilesAndFolders(paths, folder);
            } else if (executedAction == move) {
                success = destItem->project()->projectFileManager()->moveFilesAndFolders(usefulItems, folder);
            }

            if (success) {
                //expand target folder
                expand( mapFromItem(folder));

                //and select new items
                QItemSelection selection;
                for (const Path& path : std::as_const(paths)) {
                    const Path targetPath(folder->path(), path.lastPathSegment());
                    const auto folderChildren = folder->children();
                    for (ProjectBaseItem* item : folderChildren) {
                        if (item->path() == targetPath) {
                            QModelIndex indx = mapFromItem( item );
                            selection.append(QItemSelectionRange(indx, indx));
                            setCurrentIndex(indx);
                        }
                    }
                }
                selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
            }
        }
        else if (destItem->target() && destItem->project()->buildSystemManager())
        {
            QMenu dropMenu(this);

            QString seq = QKeySequence( Qt::ControlModifier ).toString();
            seq.chop(1);
            auto* addToTarget = new QAction(i18nc("@action:inmenu", "&Add to Build Target") + QLatin1Char('\t') + seq, &dropMenu);
            addToTarget->setIcon(QIcon::fromTheme(QStringLiteral("edit-link")));
            dropMenu.addAction(addToTarget);

            dropMenu.addSeparator();

            auto* cancel = new QAction(i18nc("@action:inmenu", "C&ancel") + QLatin1Char('\t') + QKeySequence(Qt::Key_Escape).toString(), &dropMenu);
            cancel->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
            dropMenu.addAction(cancel);

            QAction *executedAction = nullptr;

            Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
            if (modifiers == Qt::ControlModifier) {
                executedAction = addToTarget;
            } else {
                executedAction = dropMenu.exec(this->mapToGlobal(event->position().toPoint()));
            }
            if (executedAction == addToTarget) {
                QList<ProjectFileItem*> usefulItems = fileItemsWithin(selectionCtxt->items());
                filterDroppedItems(usefulItems, destItem);
                destItem->project()->buildSystemManager()->addFilesToTarget(usefulItems, destItem->target());
            }
        }
    }
    event->accept();
}

QModelIndex ProjectTreeView::mapFromSource(const QAbstractProxyModel* proxy, const QModelIndex& sourceIdx)
{
    const QAbstractItemModel* next = proxy->sourceModel();
    Q_ASSERT(next == sourceIdx.model() || qobject_cast<const QAbstractProxyModel*>(next));
    if(next == sourceIdx.model())
        return proxy->mapFromSource(sourceIdx);
    else {
        const auto* nextProxy = qobject_cast<const QAbstractProxyModel*>(next);
        QModelIndex idx = mapFromSource(nextProxy, sourceIdx);
        Q_ASSERT(idx.model() == nextProxy);
        return proxy->mapFromSource(idx);
    }
}

QModelIndex ProjectTreeView::mapFromItem(const ProjectBaseItem* item)
{
    QModelIndex ret = mapFromSource(qobject_cast<const QAbstractProxyModel*>(model()), item->index());
    Q_ASSERT(ret.model() == model());
    return ret;
}

void ProjectTreeView::slotActivated( const QModelIndex &index )
{
    if ( QApplication::keyboardModifiers() & Qt::CTRL || QApplication::keyboardModifiers() & Qt::SHIFT ) {
        // Do not open file when Ctrl or Shift is pressed; that's for selection
        return;
    }
    auto *item = index.data(ProjectModel::ProjectItemRole).value<ProjectBaseItem*>();
    if ( item && item->file() )
    {
        emit activate( item->file()->path() );
    }
}

void ProjectTreeView::projectClosed(KDevelop::IProject* project)
{
    if ( project == m_previousSelection )
        m_previousSelection = nullptr;
}


QList<ProjectBaseItem*> ProjectTreeView::selectedProjects()
{
    QList<ProjectBaseItem*> itemlist;
    if ( selectionModel()->hasSelection() ) {
        const QModelIndexList indexes = selectionModel()->selectedRows();
        for ( const QModelIndex& index: indexes ) {
            auto* item = index.data( ProjectModel::ProjectItemRole ).value<ProjectBaseItem*>();
            if ( item ) {
                itemlist << item;
                m_previousSelection = item->project();
            }
        }
    }

    // add previous selection if nothing is selected right now
    if ( itemlist.isEmpty() && m_previousSelection ) {
        itemlist << m_previousSelection->projectItem();
    }

    return itemlist;
}

KDevelop::IProject* ProjectTreeView::getCurrentProject()
{
    auto itemList = selectedProjects();
    if ( !itemList.isEmpty() ) {
        return itemList.at( 0 )->project();
    }
    return nullptr;
}

void ProjectTreeView::popupContextMenu( const QPoint &pos )
{
    QList<ProjectBaseItem*> itemlist;
    if ( indexAt( pos ).isValid() ) {
        itemlist = selectedProjects();
    }
    QMenu menu( this );

    KDevelop::ProjectItemContextImpl context(itemlist);
    const QList<ContextMenuExtension> extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions(&context, &menu);

    QList<QAction*> buildActions;
    QList<QAction*> vcsActions;
    QList<QAction*> analyzeActions;
    QList<QAction*> extActions;
    QList<QAction*> projectActions;
    QList<QAction*> fileActions;
    QList<QAction*> runActions;
    for (const ContextMenuExtension& ext : extensions) {
        buildActions += ext.actions(ContextMenuExtension::BuildGroup);
        fileActions += ext.actions(ContextMenuExtension::FileGroup);
        projectActions += ext.actions(ContextMenuExtension::ProjectGroup);
        vcsActions += ext.actions(ContextMenuExtension::VcsGroup);
        analyzeActions += ext.actions(ContextMenuExtension::AnalyzeProjectGroup);
        extActions += ext.actions(ContextMenuExtension::ExtensionGroup);
        runActions += ext.actions(ContextMenuExtension::RunGroup);
    }

    if ( analyzeActions.count() )
    {
        auto* analyzeMenu = new QMenu(i18nc("@title:menu", "Analyze with"), &menu);
        analyzeMenu->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok")));
        for (QAction* act : std::as_const(analyzeActions)) {
            analyzeMenu->addAction( act );
        }
        analyzeActions = {analyzeMenu->menuAction()};
    }

    popupContextMenu_appendActions(menu, buildActions);
    popupContextMenu_appendActions(menu, runActions );
    popupContextMenu_appendActions(menu, fileActions);
    popupContextMenu_appendActions(menu, vcsActions);
    popupContextMenu_appendActions(menu, analyzeActions);
    popupContextMenu_appendActions(menu, extActions);

    if (itemlist.size() == 1 && itemlist.first()->folder() && !itemlist.first()->folder()->parent()) {
        auto* projectConfig = new QAction(i18nc("@action:inmenu", "Open Configuration..."), &menu);
        projectConfig->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
        connect( projectConfig, &QAction::triggered, this, &ProjectTreeView::openProjectConfig );
        projectActions << projectConfig;
    }
    popupContextMenu_appendActions(menu, projectActions);

    if ( !menu.isEmpty() ) {
        menu.exec(viewport()->mapToGlobal(pos));
    }
}

void ProjectTreeView::openProjectConfig()
{
    if ( IProject* project = getCurrentProject() ) {
        IProjectController* ip = ICore::self()->projectController();
        ip->configureProject( project );
    }
}

void ProjectTreeView::saveState( IProject* project )
{
    // nullptr won't create a usable saved state, so spare the effort
    if ( !project ) {
        return;
    }

    KConfigGroup configGroup( ICore::self()->activeSession()->config(),
                              settingsConfigGroup() + project->name() );

    ProjectModelSaver saver;
    saver.setProject( project );
    saver.setView( this );
    saver.saveState( configGroup );
}

void ProjectTreeView::restoreState( IProject* project )
{
    if ( !project ) {
        return;
    }

    KConfigGroup configGroup( ICore::self()->activeSession()->config(),
                              settingsConfigGroup() + project->name() );
    ProjectModelSaver saver;
    saver.setProject( project );
    saver.setView( this );
    saver.restoreState( configGroup );
}

void ProjectTreeView::rowsInserted( const QModelIndex& parent, int start, int end )
{
    QTreeView::rowsInserted( parent, start, end );

    if ( !parent.model() ) {
        const auto& projects = selectedProjects();
        for (const auto& project: projects) {
            restoreState( project->project() );
        }
    }
}

void ProjectTreeView::rowsAboutToBeRemoved( const QModelIndex& parent, int start, int end )
{
    if ( !parent.model() ) {
        const auto& projects = selectedProjects();
        for (const auto& project : projects) {
            saveState( project->project() );
        }
    }

    QTreeView::rowsAboutToBeRemoved( parent, start, end );
}

void ProjectTreeView::aboutToShutdown()
{
    // save all projects, not just the selected ones
    const auto projects = ICore::self()->projectController()->projects();
    for ( const auto& project: projects ) {
        saveState( project );
    }
}

void ProjectTreeView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return && currentIndex().isValid() && state()!=QAbstractItemView::EditingState)
    {
        event->accept();
        slotActivated(currentIndex());
    }
    else
        QTreeView::keyPressEvent(event);
}

void ProjectTreeView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    if (WidgetColorizer::colorizeByProject()) {
        const auto projectPath = index.data(ProjectModel::ProjectRole).value<IProject *>()->path();
        const QColor color = WidgetColorizer::colorForId(qHash(projectPath), palette(), true);
        WidgetColorizer::drawBranches(this, painter, rect, index, color);
    }

    QTreeView::drawBranches(painter, rect, index);
}

#include "moc_projecttreeview.cpp"
