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
 
#include "treeitem.h"

#include <QStringList>
#include <QModelIndex>

#include "treemodel.h"

#include "kdebug.h"

using namespace KDevelop;

TreeItem::TreeItem(TreeModel* model, TreeItem *parent)
: model_(model), more_(false), ellipsis_(0), expanded_(false)
{
    parentItem = parent;
}

TreeItem::~TreeItem()
{
    foreach (TreeItem *it, childItems)
        delete it;
    delete ellipsis_;
}

void TreeItem::setData(const QVector<QVariant> &data)
{
    itemData=data;
}

void TreeItem::appendChild(TreeItem *item, bool initial)
{
    QModelIndex index = model_->indexForItem(this, 0);

    // Note that we need emit beginRemoveRows, even if we're replacing
    // ellipsis item with the real one.  The number of rows does not change
    // with the result, but the last item is different.  The address of the
    // item is stored inside QModelIndex, so just replacing the item, and
    // deleting the old one, will lead to crash.
    if (more_)
    {
        if (!initial)
            model_->beginRemoveRows(index, childItems.size(), childItems.size());
        more_ = false;
        delete ellipsis_;
        ellipsis_ = 0;
        if (!initial)
            model_->endRemoveRows();
    }

    if (!initial)
        model_->beginInsertRows(index, childItems.size(), childItems.size());
    childItems.append(item);
    if (!initial)
        model_->endInsertRows();
}

void TreeItem::insertChild(int position, TreeItem *child, bool initial)
{
    QModelIndex index = model_->indexForItem(this, 0);

    if (!initial)
        model_->beginInsertRows(index, position, position);
    childItems.insert(position, child);
    if (!initial)
        model_->endInsertRows();    
}

void TreeItem::reportChange()
{
    QModelIndex index = model_->indexForItem(this, 0);
    QModelIndex index2 = model_->indexForItem(this, itemData.size()-1);
    emit model_->dataChanged(index, index2);
}

void KDevelop::TreeItem::reportChange(int column)
{
    QModelIndex index = model_->indexForItem(this, column);
    emit model_->dataChanged(index, index);
}


void TreeItem::removeChild(int index)
{
    QModelIndex modelIndex = model_->indexForItem(this, 0);

    model_->beginRemoveRows(modelIndex, index, index);
    childItems.erase(childItems.begin() + index);
    model_->endRemoveRows();
}

void TreeItem::removeSelf()
{
    QModelIndex modelIndex = model_->indexForItem(this, 0);
    parentItem->removeChild(modelIndex.row());
}

void TreeItem::deleteChildren()
{
    QVector<TreeItem*> copy = childItems;
    clear();
    // Only delete the children after removing them
    // from model.  Otherwise, the model will touch
    // deleted things, with undefined results.
    for (int i = 0; i < childItems.size(); ++i) {
        TreeItem* v = child(i);
        delete v;
    }
}

void TreeItem::clear()
{
    if (childItems.size() || more_)
    {
        QModelIndex index = model_->indexForItem(this, 0);
        model_->beginRemoveRows(index, 0, childItems.size()-1+more_);
        childItems.clear();
        more_ = false;
        delete ellipsis_;
        ellipsis_ = 0;
        model_->endRemoveRows();
    }
}

TreeItem *TreeItem::child(int row)
{
    if (row < childItems.size())
        return childItems.value(row);
    else if (row == childItems.size() && more_)
        return ellipsis_;
    else
        return NULL;
       
}

int TreeItem::childCount() const
{
    return childItems.count() + more_;
}

int TreeItem::columnCount() const
{
    return itemData.count();
}

QVariant TreeItem::data(int column, int role) const
{
    if (role == Qt::DecorationRole)
        return icon(column);
    else if (role==Qt::DisplayRole || role == Qt::EditRole)
        return itemData.value(column);
    return QVariant();
}

TreeItem *TreeItem::parent()
{
    return parentItem;
}

int TreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

class EllipsisItem : public TreeItem
{
public:
    EllipsisItem(TreeModel *model, TreeItem *parent)
    : TreeItem(model, parent)
    {
        QVector<QVariant> data;
        data.push_back("...");
        for (int i = 1; i < model->columnCount(QModelIndex()); ++i)
            data.push_back("");
        setData(data);
    }

    void clicked()
    {
        kDebug() << "Ellipsis item clicked";
        /* FIXME: restore
           Q_ASSERT (parentItem->hasMore()); */
        parentItem->fetchMoreChildren();
    }

    void fetchMoreChildren() {}
};

void TreeItem::setHasMore(bool more)
{
    /* FIXME: this will crash if used in ctor of root item,
       where the model is not associated with item or something.  */
    QModelIndex index = model_->indexForItem(this, 0);

    if (more && !more_)
    {
        model_->beginInsertRows(index, childItems.size(), childItems.size());
        ellipsis_ = new EllipsisItem (model(), this);
        more_ = more;
        model_->endInsertRows();
    }
    else if (!more && more_)
    {
        model_->beginRemoveRows(index, childItems.size(), childItems.size());
        delete ellipsis_;
        ellipsis_ = 0;
        more_ = more;
        model_->endRemoveRows();
    }
}

void TreeItem::emitAllChildrenFetched()
{
    emit allChildrenFetched();
}

void TreeItem::setHasMoreInitial(bool more)
{
    more_ = more;

    if (more)
    {
        ellipsis_ = new EllipsisItem (model(), this);
    }
}

QVariant KDevelop::TreeItem::icon(int column) const
{
    Q_UNUSED(column);
    return QVariant();
}

void KDevelop::TreeItem::setExpanded(bool b)
{
    if (expanded_ != b) {
        expanded_ = b;
        if (expanded_) emit expanded();
        else emit collapsed();
    }
}

