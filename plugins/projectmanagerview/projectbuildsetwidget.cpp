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

#include <kdebug.h>
#include <kicon.h>
#include <kmenu.h>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectbuildsetmodel.h>

#include "projectmanagerviewplugin.h"
#include "projectmanagerview.h"

#include "ui_projectbuildsetwidget.h"
#include <QHeaderView>
#include <project/projectbuildsetmodel.h>

ProjectBuildSetWidget::ProjectBuildSetWidget( QWidget* parent )
    : QWidget( parent ), m_view( 0 ),
     m_ui( new Ui::ProjectBuildSetWidget )
{
    m_ui->setupUi( this );
    
    m_ui->addItemButton->setIcon( KIcon( "list-add" ) );
    connect( m_ui->addItemButton, SIGNAL( clicked() ),
             this, SLOT( addItems() ) );

    m_ui->removeItemButton->setIcon( KIcon( "list-remove" ) );
    connect( m_ui->removeItemButton, SIGNAL( clicked() ),
             this, SLOT( removeItems() ) );

    m_ui->upButton->setIcon( KIcon( "go-up" ) );
    connect( m_ui->upButton, SIGNAL( clicked() ),
             SLOT( moveUp() ) );
    
    m_ui->downButton->setIcon( KIcon( "go-down" ) );
    connect( m_ui->downButton, SIGNAL( clicked() ),
             SLOT( moveDown() ) );
    
    m_ui->topButton->setIcon( KIcon( "go-top" ) );
    connect( m_ui->topButton, SIGNAL( clicked() ),
             SLOT( moveToTop() ) );
    
    m_ui->bottomButton->setIcon( KIcon( "go-bottom" ) );
    connect( m_ui->bottomButton, SIGNAL( clicked() ),
             SLOT( moveToBottom() ) );
    
    m_ui->itemView->horizontalHeader()->setStretchLastSection(true);
    m_ui->itemView->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( m_ui->itemView, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             SLOT(showContextMenu(const QPoint&) ) );
    layout()->setMargin(0);
}

void ProjectBuildSetWidget::setProjectView( ProjectManagerView* view )
{
    m_view = view;
    m_ui->itemView->setModel( KDevelop::ICore::self()->projectController()->buildSetModel() );
    connect( m_ui->itemView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&) ),
             this, SLOT(selectionChanged()) );
}

void ProjectBuildSetWidget::selectionChanged()
{
    QModelIndexList selectedRows = m_ui->itemView->selectionModel()->selectedRows();
    kDebug() << "checking selectionmodel:" << selectedRows;
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

void ProjectBuildSetWidget::showContextMenu( const QPoint& p )
{
    if( m_ui->itemView->selectionModel()->selectedRows().isEmpty() )
        return;

    KMenu m;
    m.setTitle( i18n("Buildset") );
    m.addAction( i18n( "Remove from buildset" ), this, SLOT( removeItems() ) );
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
    kDebug() << "removing:" << range.top() << range.height();
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

