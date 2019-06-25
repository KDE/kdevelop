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

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QVector>
#include <QString>

#include <debugger/debuggerexport.h>

namespace KDevelop {

class TreeItem;
class TreeModelPrivate;

class KDEVPLATFORMDEBUGGER_EXPORT TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TreeModel(const QVector<QString>& headers, QObject *parent = nullptr);
    void setRootItem(TreeItem *item);
    ~TreeModel() override;

    void expanded(const QModelIndex &index);
    void collapsed(const QModelIndex &index);
    void clicked(const QModelIndex &index);

    void setEditable(bool);
    TreeItem* root() const;

    enum {
        ItemRole = Qt::UserRole,
    };

public: // QAbstractItemModel overrides
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role) override;

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
    const QScopedPointer<class TreeModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(TreeModel)
};

}

#endif
