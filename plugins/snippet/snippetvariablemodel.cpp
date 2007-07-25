/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "snippetvariablemodel.h"

#include <klocale.h>


QString SnippetVariableItem::data(int column)
{
    if (column == 0) {
        return name_;
    } else if (column == 1) {
        return value_;
    }

    return QString();
}


SnippetVariableModel::SnippetVariableModel( QObject * parent )
 : QAbstractItemModel( parent )
{
}


SnippetVariableModel::~SnippetVariableModel()
{
}

int SnippetVariableModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return variables_.count();
}

int SnippetVariableModel::columnCount(const QModelIndex & parent) const
{
  Q_UNUSED(parent);
  return 2;
}

QModelIndex SnippetVariableModel::index(int row, int column, const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return createIndex(row, column, variables_.at(row) );
}

QModelIndex SnippetVariableModel::parent(const QModelIndex & index) const
{
    Q_UNUSED(index);

    return QModelIndex();
}

bool SnippetVariableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);

    if (!index.isValid())
        return false;

    SnippetVariableItem* item = reinterpret_cast<SnippetVariableItem*>(index.internalPointer());
    if (item) {
        if ( index.column() == 1 ) {
            item->setValue( value.toString() );
            return true;
        }
    }

    return false;
}

QVariant SnippetVariableModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return false;

    SnippetVariableItem* item = reinterpret_cast<SnippetVariableItem*>(index.internalPointer());
    if (item) {
        switch (role) {
            case Qt::DisplayRole: return item->data( index.column() );
            default: break;
        }
    }

    return QVariant();
}

void SnippetVariableModel::appendVariable(const QString& name)
{
    QModelIndex parent;

    int row = variables_.count();

    beginInsertRows(parent, row, row);
    SnippetVariableItem* var = new SnippetVariableItem( name );
    variables_.append( var );
    endInsertRows();
}

Qt::ItemFlags SnippetVariableModel::flags(const QModelIndex & index) const
{
    if (!index.isValid())
        return false;

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() == 1) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

QVariant SnippetVariableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return QString( i18n("Variable") );
        } else if (section == 1) {
            return QString( i18n("Value") );
        }
    }

    if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        return QString::number( section+1 );
    }

    return QVariant();
}

// kate: space-indent on; indent-width 4; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
