/*
 * This file is part of KDevelop
 *
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
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

#ifndef KDEVPLATFORM_TREEMODEL_H
#define KDEVPLATFORM_TREEMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QModelIndex>
#include <QtCore/QVariant>
#include <QtCore/QVector>
#include <QtCore/QString>

#include "../debuggerexport.h"

namespace KDevelop {

class TreeItem;

class KDEVPLATFORMDEBUGGER_EXPORT TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(const QVector<QString>& headers, QObject *parent = 0);
    void setRootItem(TreeItem *item);
    ~TreeModel();

    void expanded(const QModelIndex &index);
    void collapsed(const QModelIndex &index);
    void clicked(const QModelIndex &index);

    void setEditable(bool);
    TreeItem* root() const;

public: // QAbstractItemModel overrides
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role);

Q_SIGNALS:
    void itemChildrenReady();

public:
    TreeItem* itemForIndex(const QModelIndex& index) const;
    QModelIndex indexForItem(TreeItem *item, int column) const;

    using QAbstractItemModel::beginInsertRows;
    using QAbstractItemModel::endInsertRows;
    using QAbstractItemModel::beginRemoveRows;
    using QAbstractItemModel::endRemoveRows;
    using QAbstractItemModel::dataChanged;

private:
    QVector<QString> headers_;
    TreeItem *root_;
    bool editable_;
};

}

#endif
