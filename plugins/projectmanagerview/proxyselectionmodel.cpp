/* This file is part of KDevelop
    Copyright 2008 Andreas Pakulat <apaku@gmx.de>

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

#include "proxyselectionmodel.h"
#include <QAbstractProxyModel>
#include <QAbstractItemView>

ProxySelectionModel::ProxySelectionModel( QAbstractItemView* itemview, QItemSelectionModel* sourceSelectionModel, QObject* parent )
    : QItemSelectionModel( itemview->model(), parent ), proxyModel(0), selectionModel( sourceSelectionModel ), view( itemview ), doingUpdate( false )
{
    proxyModel = dynamic_cast<QAbstractProxyModel*>( view->model() );
    if( proxyModel )
    {
        connect( selectionModel, SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
                 this, SLOT(changeCurrent( const QModelIndex&, const QModelIndex& ) ) );
        connect( selectionModel, SIGNAL( currentColumnChanged( const QModelIndex&, const QModelIndex& ) ),
                 this, SLOT(changeCurrentColumn( const QModelIndex&, const QModelIndex& ) ) );
        connect( selectionModel, SIGNAL( currentRowChanged( const QModelIndex&, const QModelIndex& ) ),
                 this, SLOT(changeCurrentRow( const QModelIndex&, const QModelIndex& ) ) );
        connect( selectionModel, SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
                 this, SLOT(changeSelection( const QItemSelection&, const QItemSelection& ) ) );
    
        connect( this, SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
                 this, SLOT(forwardChangeCurrent( const QModelIndex&, const QModelIndex& ) ) );
        connect( this, SIGNAL( currentColumnChanged( const QModelIndex&, const QModelIndex& ) ),
                 this, SLOT(forwardChangeCurrentColumn( const QModelIndex&, const QModelIndex& ) ) );
        connect( this, SIGNAL( currentRowChanged( const QModelIndex&, const QModelIndex& ) ),
                 this, SLOT(forwardChangeCurrentRow( const QModelIndex&, const QModelIndex& ) ) );
        connect( this, SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
                 this, SLOT(forwardChangeSelection( const QItemSelection&, const QItemSelection& ) ) );
    }
    view->setSelectionModel(this);
}

void ProxySelectionModel::changeCurrent( const QModelIndex& current, const QModelIndex& previous )
{
    if( doingUpdate )
        return;
    doingUpdate = true;
    emit currentChanged( proxyModel->mapFromSource( current ), proxyModel->mapFromSource( previous ) );
    doingUpdate = false;
}

void ProxySelectionModel::changeCurrentColumn( const QModelIndex& current, const QModelIndex& previous )
{
    if( doingUpdate )
        return;
    doingUpdate = true;
    emit currentColumnChanged( proxyModel->mapFromSource( current ), proxyModel->mapFromSource( previous ) );
    doingUpdate = false;
}

void ProxySelectionModel::changeCurrentRow( const QModelIndex& current, const QModelIndex& previous )
{
    if( doingUpdate )
        return;
    doingUpdate = true;
    emit currentRowChanged( proxyModel->mapFromSource( current ), proxyModel->mapFromSource( previous ) );
    doingUpdate = false;
}

void ProxySelectionModel::changeSelection( const QItemSelection& selected, const QItemSelection& deselected )
{
    if( doingUpdate )
        return;
    doingUpdate = true;
    emit selectionChanged( proxyModel->mapSelectionFromSource( selected ), proxyModel->mapSelectionFromSource( deselected ) );
    doingUpdate = false;
}

QItemSelectionModel::SelectionFlags ProxySelectionModel::selectionFlags() const
{
    QItemSelectionModel::SelectionFlags f;
    if( view->selectionMode() == QAbstractItemView::SingleSelection )
    {
      f = QItemSelectionModel::ClearAndSelect;
      if( view->selectionBehavior() == QAbstractItemView::SelectItems )
          f |= QItemSelectionModel::NoUpdate;
      if( view->selectionBehavior() == QAbstractItemView::SelectRows )
          f |= QItemSelectionModel::Rows;
      if( view->selectionBehavior() == QAbstractItemView::SelectColumns )
          f |= QItemSelectionModel::Columns;
    }
    else
          f = QItemSelectionModel::NoUpdate;
    return f;
}

void ProxySelectionModel::forwardChangeCurrent( const QModelIndex& current, const QModelIndex& previous )
{
    if( doingUpdate )
        return;
    doingUpdate = true;
    selectionModel->setCurrentIndex( current, selectionFlags() );
    doingUpdate = false;
}

void ProxySelectionModel::forwardChangeCurrentColumn( const QModelIndex& current, const QModelIndex& previous )
{
    if( doingUpdate )
        return;
    doingUpdate = true;
    selectionModel->setCurrentIndex( current, selectionFlags() );
    doingUpdate = false;
}

void ProxySelectionModel::forwardChangeCurrentRow( const QModelIndex& current, const QModelIndex& previous )
{
    if( doingUpdate )
        return;
    doingUpdate = true;
    selectionModel->setCurrentIndex( current, selectionFlags() );
    doingUpdate = false;
}

void ProxySelectionModel::forwardChangeSelection( const QItemSelection& selected, const QItemSelection& deselected )
{
    if( doingUpdate )
        return;
    doingUpdate = true;
    selectionModel->select( selected, QItemSelectionModel::Select );
    selectionModel->select( deselected, QItemSelectionModel::Deselect );
    doingUpdate = false;
}


#include "proxyselectionmodel.moc"

