/*
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: GPL-2.0-or-later
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
