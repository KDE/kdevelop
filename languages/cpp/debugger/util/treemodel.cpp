
#include <QtGui>

#include <iostream>

#include "treeitem.h"
#include "treemodel.h"

using namespace GDBDebugger;

TreeModel::TreeModel(const QVector<QString>& headers,
                     QObject *parent)
  : QAbstractItemModel(parent), headers_(headers), root_(NULL)
{
}

void TreeModel::setRootItem(TreeItem *item)
{
    root_ = item;
    root_->fetchMoreChildren();
}

TreeModel::~TreeModel()
{
    delete root_;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    return headers_.size();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{ 
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return headers_[section];

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent)
    const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = root_;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return createIndex(row, column, 0);
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == root_)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = root_;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

TreeItem* TreeModel::itemForIndex(const QModelIndex& index) const
{
    if (!index.isValid())
        return root_;
    else
        return static_cast<TreeItem*>(index.internalPointer());
}

QModelIndex TreeModel::indexForItem(TreeItem *item, int column) const
{
    if (item->parent() == 0)
        return QModelIndex();

    if (TreeItem* parent = item->parent()) 
    {
        /* FIXME: we might store row directly in item.  */
        int row = parent->childItems.indexOf(item);
        Q_ASSERT(row != -1);

        return createIndex(row, column, item);
    }
    else 
    {
        return QModelIndex();
    }        
}

void TreeModel::expanded(const QModelIndex &index)
{
    TreeItem* item = itemForIndex(index);
    if (item->hasMore() && item->childCount() == 1)
        item->fetchMoreChildren();
    item->setExpanded(true);
}

void TreeModel::collapsed(const QModelIndex &index)
{
    TreeItem* item = itemForIndex(index);
    item->setExpanded(false);
}

void TreeModel::clicked(const QModelIndex &index)
{
    TreeItem* item = itemForIndex(index);
    item->clicked();
}
