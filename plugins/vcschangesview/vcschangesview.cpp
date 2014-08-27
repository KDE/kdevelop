/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include <vcs/models/projectchangesmodel.h>
#include <project/projectmodel.h>

#include <QIcon>

#include <KActionCollection>
#include <KMenu>
#include <KLocalizedString>

#include "vcschangesviewplugin.h"

using namespace KDevelop;

VcsChangesView::VcsChangesView(VcsProjectIntegrationPlugin* plugin, QWidget* parent)
    : QTreeView(parent)
{
    setRootIsDecorated(false);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(ContiguousSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setTextElideMode(Qt::ElideLeft);
    setWordWrap(true);
    
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(popupContextMenu(QPoint)));
    
    foreach(QAction* action, plugin->actionCollection()->actions())
        addAction(action);
    
    QAction* action = plugin->actionCollection()->action("locate_document");
    connect(action, SIGNAL(triggered(bool)), SLOT(selectCurrentDocument()));
    connect(this, SIGNAL(doubleClicked(QModelIndex)), SLOT(openSelected(QModelIndex)));
}

static void appendActions(KMenu* menu, const QList<QAction*>& actions)
{
    menu->addSeparator();
    menu->addActions(actions);
}

void VcsChangesView::popupContextMenu( const QPoint &pos )
{
    KUrl::List urls;
    QList<IProject*> projects;
    QModelIndexList selectionIdxs = selectedIndexes();
    if(selectionIdxs.isEmpty())
        return;
    
    foreach(const QModelIndex& idx, selectionIdxs) {
        if(idx.column()==0) {
            if(idx.parent().isValid())
                urls += idx.data(KDevelop::VcsFileChangesModel::VcsStatusInfoRole).value<VcsStatusInfo>().url();
            else {
                projects += ICore::self()->projectController()->findProjectByName(idx.data(ProjectChangesModel::ProjectNameRole).toString());
            }
        }
    }

    QPointer<KMenu> menu = new KMenu;
    QAction* refreshAction = menu->addAction(QIcon::fromTheme("view-refresh"), i18n("Refresh"));
    QList<ContextMenuExtension> extensions;
    if(!urls.isEmpty()) {
        KDevelop::FileContext context(urls);
        extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( &context );
    } else {
        QList<ProjectBaseItem*> items;
        foreach(IProject* p, projects)
            items += p->projectItem();
        
        KDevelop::ProjectItemContext context(items);
        extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( &context );
        
    }

    QList<QAction*> buildActions;
    QList<QAction*> vcsActions;
    QList<QAction*> extActions;
    QList<QAction*> projectActions;
    QList<QAction*> fileActions;
    QList<QAction*> runActions;
    foreach( const ContextMenuExtension& ext, extensions )
    {
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
        QAction* res = menu->exec( mapToGlobal( pos ) );
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
    
    KUrl url = doc->url();
    IProject* p = ICore::self()->projectController()->findProjectForUrl(url);
    QStandardItem* item = 0;
    
    if(p) {
        ProjectChangesModel* pcmodel = static_cast<ProjectChangesModel*>(model());
        item = pcmodel->fileItemForUrl(pcmodel->projectItem(p), url);
    }
    
    if(item) {
        expand(item->index().parent());
        setCurrentIndex(item->index());
    } else
        collapseAll();
}

void VcsChangesView::setModel(QAbstractItemModel* model)
{
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(expand(QModelIndex)));
    QTreeView::setModel(model);
}

void VcsChangesView::openSelected(const QModelIndex& index)
{
    if(!index.parent().isValid()) //then it's a project
        return;
    QModelIndex idx = index.sibling(index.row(), 0);
    VcsStatusInfo info = idx.data(ProjectChangesModel::VcsStatusInfoRole).value<VcsStatusInfo>();
    KUrl url = info.url();
    
    ICore::self()->documentController()->openDocument(url);
}
