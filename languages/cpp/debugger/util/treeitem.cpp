
#include <QStringList>
#include <QModelIndex>

#include "treeitem.h"
#include "treemodel.h"

using namespace GDBDebugger;

TreeItem::TreeItem(TreeModel* model, TreeItem *parent)
  : model_(model), more_(false), ellipsis_(0)
{
    parentItem = parent;
}

TreeItem::~TreeItem()
{
    foreach (TreeItem *it, childItems)
        delete it;
    delete ellipsis_;
}

void TreeItem::setData(const QVector<QString> &data)
{
    itemData.clear();
    foreach (const QString &s, data) {
        itemData.push_back(s);
    }
}

void TreeItem::appendChild(TreeItem *item)
{
    QModelIndex index = model_->indexForItem(this, 0);
    QModelIndex index2 = model_->indexForItem(this, itemData.size()-1);

    if (more_)
    {
        /* No need to call beginInsertRows, as we're
           essentially replacing "..." with this new
           item.  */
        childItems.append(item);
        emit model_->dataChanged(index, index2);
        more_ = false;
    }
    else
    {
        model_->beginInsertRows(index, childItems.size(), childItems.size());
        childItems.append(item);
        model_->endInsertRows();
    }
}

void TreeItem::reportChange()
{
    QModelIndex index = model_->indexForItem(this, 0);
    QModelIndex index2 = model_->indexForItem(this, itemData.size()-1);
    emit model_->dataChanged(index, index2);
}

void TreeItem::removeChild(int index)
{
    QModelIndex modelIndex = model_->indexForItem(this, 0);

    model_->beginRemoveRows(modelIndex, index, index);
    childItems.erase(childItems.begin() + index);
    model_->endRemoveRows();
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

QVariant TreeItem::data(int column) const
{
    return itemData.value(column);
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
        QVector<QString> data;
        data.push_back("...");
        for (int i = 1; i < model->columnCount(QModelIndex()); ++i)
            data.push_back("");
        setData(data);
    }

    void clicked()
    {
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
        more_ = more;
        model_->endRemoveRows();
    }
}

void TreeItem::setHasMoreInitial(bool more)
{
    more_ = more;

    if (more)
    {
        ellipsis_ = new EllipsisItem (model(), this);
    }
}

