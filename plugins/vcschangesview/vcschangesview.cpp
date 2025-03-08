/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcschangesview.h"
#include <interfaces/icore.h>
#include <interfaces/context.h>
#include <interfaces/iproject.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/contextmenuextension.h>
#include <vcs/vcsstatusinfo.h>
#include <project/projectchangesmodel.h>
#include <project/projectmodel.h>
#include <project/projectutils.h>

#include <QIcon>
#include <QMenu>
#include <QPointer>

#include <KActionCollection>
#include <KLocalizedString>

#include "vcschangesviewplugin.h"

using namespace KDevelop;

VcsChangesView::VcsChangesView(VcsProjectIntegrationPlugin* plugin, QWidget* parent)
    : QTreeView(parent)
{
    setRootIsDecorated(false);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(ExtendedSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setUniformRowHeights(true);
    setTextElideMode(Qt::ElideLeft);
    setWindowIcon(QIcon::fromTheme(QStringLiteral("exchange-positions"), windowIcon()));
    
    connect(this, &VcsChangesView::customContextMenuRequested, this, &VcsChangesView::popupContextMenu);

    const auto pluginActions = plugin->actionCollection()->actions();
    for (QAction* action : pluginActions) {
        addAction(action);
    }
    
    QAction* action = plugin->actionCollection()->action(QStringLiteral("locate_document"));
    connect(action, &QAction::triggered, this, &VcsChangesView::selectCurrentDocument);
    connect(this, &VcsChangesView::doubleClicked, this, &VcsChangesView::openSelected);
}

static void appendActions(QMenu* menu, const QList<QAction*>& actions)
{
    menu->addSeparator();
    menu->addActions(actions);
}

void VcsChangesView::popupContextMenu( const QPoint &pos )
{
    QList<QUrl> urls;
    QList<IProject*> projects;
    const QModelIndexList selectionIdxs = selectedIndexes();
    if(selectionIdxs.isEmpty())
        return;
    
    for (const QModelIndex& idx : selectionIdxs) {
        if(idx.column()==0) {
            if(idx.parent().isValid())
                urls += idx.data(KDevelop::VcsFileChangesModel::VcsStatusInfoRole).value<VcsStatusInfo>().url();
            else {
                IProject* project = ICore::self()->projectController()->findProjectByName(idx.data(ProjectChangesModel::ProjectNameRole).toString());
                if (project) {
                    projects += project;
                } else {
                    qWarning() << "Couldn't find a project for project: " << idx.data(ProjectChangesModel::ProjectNameRole).toString();
                }
            }
        }
    }

    QPointer<QMenu> menu = new QMenu(this);
    QAction* refreshAction = menu->addAction(QIcon::fromTheme(QStringLiteral("view-refresh")), i18nc("@action:inmenu", "Refresh"));
    QList<ContextMenuExtension> extensions;
    if(!urls.isEmpty()) {
        KDevelop::FileContext context(urls);
        extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions(&context, menu);
    } else {
        QList<ProjectBaseItem*> items;
        items.reserve(projects.size());
        for (IProject* p : std::as_const(projects)) {
            items += p->projectItem();
        }

        KDevelop::ProjectItemContextImpl context(items);
        extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions(&context, menu);
        
    }

    QList<QAction*> buildActions;
    QList<QAction*> vcsActions;
    QList<QAction*> extActions;
    QList<QAction*> projectActions;
    QList<QAction*> fileActions;
    QList<QAction*> runActions;
    for (const ContextMenuExtension& ext : std::as_const(extensions)) {
        buildActions += ext.actions(ContextMenuExtension::BuildGroup);
        fileActions += ext.actions(ContextMenuExtension::FileGroup);
        projectActions += ext.actions(ContextMenuExtension::ProjectGroup);
        vcsActions += ext.actions(ContextMenuExtension::VcsGroup);
        extActions += ext.actions(ContextMenuExtension::ExtensionGroup);
        runActions += ext.actions(ContextMenuExtension::RunGroup);
    }

    appendActions(menu, buildActions);
    appendActions(menu, runActions );
    appendActions(menu, fileActions);
    appendActions(menu, vcsActions);
    appendActions(menu, extActions);
    appendActions(menu, projectActions);

    if ( !menu->isEmpty() ) {
        QAction* res = menu->exec(viewport()->mapToGlobal(pos));
        if(res == refreshAction) {
            if(!urls.isEmpty())
                emit reload(urls);
            else
                emit reload(projects);
        }
    }
    delete menu;
}

void VcsChangesView::selectCurrentDocument()
{
    IDocument* doc = ICore::self()->documentController()->activeDocument();
    if(!doc)
        return;
    
    QUrl url = doc->url();
    QModelIndex idx;
    if (const auto* const project = ICore::self()->projectController()->findProjectForUrl(url)) {
        if (const auto* const projectModel = qobject_cast<ProjectChangesModel*>(model())) {
            if (const auto* const projectItem = projectModel->projectItem(project)) {
                idx = ProjectChangesModel::statusIndexForUrl(*projectModel, projectItem->index(), url);
            }
        }
    }

    if(idx.isValid()) {
        expand(idx.parent());
        setCurrentIndex(idx);
    } else
        collapseAll();
}

void VcsChangesView::setModel(QAbstractItemModel* model)
{
    connect(model, &QAbstractItemModel::rowsInserted, this, &VcsChangesView::expand);
    QTreeView::setModel(model);
}

void VcsChangesView::openSelected(const QModelIndex& index)
{
    if(!index.parent().isValid()) //then it's a project
        return;
    QModelIndex idx = index.sibling(index.row(), 1);
    VcsStatusInfo info = idx.data(ProjectChangesModel::VcsStatusInfoRole).value<VcsStatusInfo>();
    QUrl url = info.url();
    
    ICore::self()->documentController()->openDocument(url);
}

#include "moc_vcschangesview.cpp"
