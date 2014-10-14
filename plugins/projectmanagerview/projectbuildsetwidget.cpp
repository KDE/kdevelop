/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "projectbuildsetwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeView>
#include <QToolButton>
#include <QItemSelectionModel>
#include <QStringListModel>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QIcon>
#include <QMenu>
#include <QHeaderView>

#include <KLocalizedString>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectbuildsetmodel.h>

#include "projectmanagerviewplugin.h"
#include "projectmanagerview.h"

#include "ui_projectbuildsetwidget.h"
#include "debug.h"

#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <interfaces/iplugincontroller.h>

ProjectBuildSetWidget::ProjectBuildSetWidget( QWidget* parent )
    : QWidget( parent ), m_view( 0 ),
     m_ui( new Ui::ProjectBuildSetWidget )
{
    m_ui->setupUi( this );

    m_ui->addItemButton->setIcon( QIcon::fromTheme( "list-add" ) );
    connect( m_ui->addItemButton, SIGNAL(clicked()),
             this, SLOT(addItems()) );

    m_ui->removeItemButton->setIcon( QIcon::fromTheme( "list-remove" ) );
    connect( m_ui->removeItemButton, SIGNAL(clicked()),
             this, SLOT(removeItems()) );

    m_ui->upButton->setIcon( QIcon::fromTheme( "go-up" ) );
    connect( m_ui->upButton, SIGNAL(clicked()),
             SLOT(moveUp()) );

    m_ui->downButton->setIcon( QIcon::fromTheme( "go-down" ) );
    connect( m_ui->downButton, SIGNAL(clicked()),
             SLOT(moveDown()) );

    m_ui->topButton->setIcon( QIcon::fromTheme( "go-top" ) );
    connect( m_ui->topButton, SIGNAL(clicked()),
             SLOT(moveToTop()) );

    m_ui->bottomButton->setIcon( QIcon::fromTheme( "go-bottom" ) );
    connect( m_ui->bottomButton, SIGNAL(clicked()),
             SLOT(moveToBottom()) );

    m_ui->itemView->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( m_ui->itemView, SIGNAL(customContextMenuRequested(QPoint)),
             SLOT(showContextMenu(QPoint)) );
    layout()->setMargin(0);
}

void ProjectBuildSetWidget::setProjectView( ProjectManagerView* view )
{
    m_view = view;
    m_ui->itemView->setModel( KDevelop::ICore::self()->projectController()->buildSetModel() );
    connect( m_ui->itemView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(selectionChanged()) );
}

void ProjectBuildSetWidget::selectionChanged()
{
    QModelIndexList selectedRows = m_ui->itemView->selectionModel()->selectedRows();
    qCDebug(PLUGIN_PROJECTMANAGERVIEW) << "checking selectionmodel:" << selectedRows;
    m_ui->removeItemButton->setEnabled( !selectedRows.isEmpty() );
    m_ui->addItemButton->setEnabled( !m_view->selectedItems().isEmpty() );

    bool enableUp = selectedRows.count() > 0 && selectedRows.first().row() != 0;
    bool enableDown = selectedRows.count() > 0 && selectedRows.last().row() != m_ui->itemView->model()->rowCount() - 1;
    m_ui->upButton->setEnabled( enableUp );
    m_ui->downButton->setEnabled( enableDown );
    m_ui->bottomButton->setEnabled( enableDown );
    m_ui->topButton->setEnabled( enableUp );
}

ProjectBuildSetWidget::~ProjectBuildSetWidget()
{
    delete m_ui;
}

//TODO test whether this could be replaced by projecttreeview.cpp::popupContextMenu_appendActions
void showContextMenu_appendActions(QMenu& menu, const QList<QAction*>& actions)
{
    menu.addSeparator();
    foreach( QAction* act, actions )
    {
        menu.addAction(act);
    }
}

void ProjectBuildSetWidget::showContextMenu( const QPoint& p )
{
    if( m_ui->itemView->selectionModel()->selectedRows().isEmpty() )
        return;

    QList<KDevelop::ProjectBaseItem*> itemlist;

    if(m_ui->itemView->selectionModel()->selectedRows().count() == 1)
    {
        KDevelop::ProjectBuildSetModel* buildSet = KDevelop::ICore::self()->projectController()->buildSetModel();

        int row = m_ui->itemView->selectionModel()->selectedRows()[0].row();
        if(row < buildSet->items().size())
        {
            KDevelop::ProjectBaseItem* item = buildSet->items()[row].findItem();
            if(item)
                itemlist << item;
        }
    }


    QMenu m;
    m.setTitle( i18n("Build Set") );
    m.addAction( QIcon::fromTheme("list-remove"), i18n( "Remove From Build Set" ), this, SLOT(removeItems()) );

    if( !itemlist.isEmpty() )
    {
        KDevelop::ProjectItemContext context(itemlist);
        QList<KDevelop::ContextMenuExtension> extensions = KDevelop::ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( &context );

        QList<QAction*> buildActions;
        QList<QAction*> vcsActions;
        QList<QAction*> extActions;
        QList<QAction*> projectActions;
        QList<QAction*> fileActions;
        QList<QAction*> runActions;
        foreach( const KDevelop::ContextMenuExtension& ext, extensions )
        {
            buildActions += ext.actions(KDevelop::ContextMenuExtension::BuildGroup);
            fileActions += ext.actions(KDevelop::ContextMenuExtension::FileGroup);
            projectActions += ext.actions(KDevelop::ContextMenuExtension::ProjectGroup);
            vcsActions += ext.actions(KDevelop::ContextMenuExtension::VcsGroup);
            extActions += ext.actions(KDevelop::ContextMenuExtension::ExtensionGroup);
            runActions += ext.actions(KDevelop::ContextMenuExtension::RunGroup);
        }

        showContextMenu_appendActions(m, buildActions);
        showContextMenu_appendActions(m, runActions);
        showContextMenu_appendActions(m, fileActions);

        showContextMenu_appendActions(m, vcsActions);
        showContextMenu_appendActions(m, extActions);

        showContextMenu_appendActions(m, projectActions);
    }

    m.exec( m_ui->itemView->viewport()->mapToGlobal( p ) );
}

void ProjectBuildSetWidget::addItems()
{
    foreach( KDevelop::ProjectBaseItem* item, m_view->selectedItems() )
    {
        KDevelop::ICore::self()->projectController()->buildSetModel()->addProjectItem( item );
    }
}

void ProjectBuildSetWidget::removeItems()
{
    // We only support contigous selection, so we only need to remove the first range
    QItemSelectionRange range = m_ui->itemView->selectionModel()->selection().first();
    int top = range.top();
    qCDebug(PLUGIN_PROJECTMANAGERVIEW) << "removing:" << range.top() << range.height();
    KDevelop::ProjectBuildSetModel* buildSet = KDevelop::ICore::self()->projectController()->buildSetModel();
    buildSet->removeRows( range.top(), range.height() );
    top = qMin( top, buildSet->rowCount() - 1 );
    QModelIndex sidx = buildSet->index( top, 0 );
    QModelIndex eidx = buildSet->index( top, buildSet->columnCount() - 1 );
    m_ui->itemView->selectionModel()->select( QItemSelection( sidx, eidx ),
                                              QItemSelectionModel::ClearAndSelect );
    m_ui->itemView->selectionModel()->setCurrentIndex( sidx,  QItemSelectionModel::Current );
}

void ProjectBuildSetWidget::moveDown()
{
    // We only support contigous selection, so we only need to remove the first range
    QItemSelectionRange range = m_ui->itemView->selectionModel()->selection().first();
    int top = range.top(), height = range.height();
    KDevelop::ProjectBuildSetModel* buildSet = KDevelop::ICore::self()->projectController()->buildSetModel();
    buildSet->moveRowsDown( top, height );
    int columnCount = buildSet->columnCount();
    QItemSelection newrange( buildSet->index( top + 1, 0 ),
                             buildSet->index( top + height, columnCount - 1 ) );
    m_ui->itemView->selectionModel()->select( newrange, QItemSelectionModel::ClearAndSelect );
    m_ui->itemView->selectionModel()->setCurrentIndex( newrange.first().topLeft(),
                                                       QItemSelectionModel::Current );
}

void ProjectBuildSetWidget::moveToBottom()
{
    // We only support contigous selection, so we only need to remove the first range
    QItemSelectionRange range = m_ui->itemView->selectionModel()->selection().first();
    int top = range.top(), height = range.height();
    KDevelop::ProjectBuildSetModel* buildSet = KDevelop::ICore::self()->projectController()->buildSetModel();
    buildSet->moveRowsToBottom( top, height );
    int rowCount = buildSet->rowCount();
    int columnCount = buildSet->columnCount();
    QItemSelection newrange( buildSet->index( rowCount - height, 0 ),
                             buildSet->index( rowCount - 1, columnCount - 1 ) );
    m_ui->itemView->selectionModel()->select( newrange, QItemSelectionModel::ClearAndSelect );
    m_ui->itemView->selectionModel()->setCurrentIndex( newrange.first().topLeft(),
                                                       QItemSelectionModel::Current );
}

void ProjectBuildSetWidget::moveUp()
{
    // We only support contigous selection, so we only need to remove the first range
    QItemSelectionRange range = m_ui->itemView->selectionModel()->selection().first();
    int top = range.top(), height = range.height();
    KDevelop::ProjectBuildSetModel* buildSet = KDevelop::ICore::self()->projectController()->buildSetModel();
    buildSet->moveRowsUp( top, height );
    int columnCount = buildSet->columnCount();
    QItemSelection newrange( buildSet->index( top - 1, 0 ),
                             buildSet->index( top - 2 + height, columnCount - 1 ) );
    m_ui->itemView->selectionModel()->select( newrange, QItemSelectionModel::ClearAndSelect );
    m_ui->itemView->selectionModel()->setCurrentIndex( newrange.first().topLeft(),
                                                       QItemSelectionModel::Current );
}


void ProjectBuildSetWidget::moveToTop()
{
    // We only support contigous selection, so we only need to remove the first range
    QItemSelectionRange range = m_ui->itemView->selectionModel()->selection().first();
    int top = range.top(), height = range.height();
    KDevelop::ProjectBuildSetModel* buildSet = KDevelop::ICore::self()->projectController()->buildSetModel();
    buildSet->moveRowsToTop( top, height );
    int columnCount = buildSet->columnCount();
    QItemSelection newrange( buildSet->index( 0, 0 ),
                             buildSet->index( height - 1, columnCount - 1 ) );
    m_ui->itemView->selectionModel()->select( newrange, QItemSelectionModel::ClearAndSelect );
    m_ui->itemView->selectionModel()->setCurrentIndex( newrange.first().topLeft(),
                                                       QItemSelectionModel::Current );
}

