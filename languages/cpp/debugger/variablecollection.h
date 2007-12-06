/*
 * GDB Debugger Support
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
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

#ifndef _VARIABLECOLLECTION_H_
#define _VARIABLECOLLECTION_H_

#include <QAbstractItemModel>

#include "mi/gdbmi.h"

namespace GDBDebugger
{

class GDBController;
class VariableItem;

class VariableCollection : public QAbstractItemModel
{
    Q_OBJECT

public:
    VariableCollection(GDBController* parent);
    virtual ~VariableCollection();

    GDBController* controller() const;
    
    void addItem(VariableItem* item);
    void deleteItem(VariableItem* item);

    // Item model reimplementations
    virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags( const QModelIndex & index ) const;
    //virtual bool hasChildren( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex & index ) const;
    virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;
    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

private:
    VariableItem* parentForIndex(const QModelIndex& index) const;
    VariableItem* itemForIndex(const QModelIndex& index) const;
  
    QList<VariableItem*> m_items;
};

}

#endif
