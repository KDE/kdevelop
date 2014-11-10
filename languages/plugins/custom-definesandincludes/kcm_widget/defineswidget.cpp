/************************************************************************
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include "defineswidget.h"

#include <KAction>

#include "ui_defineswidget.h"
#include "definesmodel.h"
#include "debugarea.h"

using namespace KDevelop;

DefinesWidget::DefinesWidget( QWidget* parent )
    : QWidget ( parent ), ui( new Ui::DefinesWidget )
    , definesModel( new DefinesModel( this ) )
{
    ui->setupUi( this );
    ui->defines->setModel( definesModel );
    ui->defines->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
    connect( definesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(definesChanged()) );
    connect( definesModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(definesChanged()) );
    connect( definesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(definesChanged()) );

    KAction* delDefAction = new KAction( i18n("Delete Define"), this );
    delDefAction->setShortcut( KShortcut( "Del" ) );
    delDefAction->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    delDefAction->setIcon( KIcon("list-remove") );
    ui->defines->addAction( delDefAction );
    ui->defines->setContextMenuPolicy( Qt::ActionsContextMenu );
    connect( delDefAction, SIGNAL(triggered()), SLOT(deleteDefine()) );
}

void DefinesWidget::setDefines( const Defines& defines )
{
    bool b = blockSignals( true );
    clear();
    definesModel->setDefines( defines );
    blockSignals( b );
}

void DefinesWidget::definesChanged()
{
    definesAndIncludesDebug() << "defines changed";
    emit definesChanged( definesModel->defines() );
}

void DefinesWidget::clear()
{
    definesModel->setDefines( {} );
}

void DefinesWidget::deleteDefine()
{
    definesAndIncludesDebug() << "Deleting defines";
    QModelIndexList selection = ui->defines->selectionModel()->selectedRows();
    foreach( const QModelIndex& row, selection ) {
        definesModel->removeRow( row.row() );
    }
}

#include "defineswidget.moc"
