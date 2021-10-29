/*
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
 
#ifndef KDEVPLATFORM_TREEITEM_H
#define KDEVPLATFORM_TREEITEM_H

#include <QVariant>
#include <QVector>
#include <QIcon>

#include <iostream>

#include <debugger/debuggerexport.h>

namespace KDevelop {

class TreeModel;

class KDEVPLATFORMDEBUGGER_EXPORT TreeItem: public QObject
{
    Q_OBJECT
public:
    ~TreeItem() override;

// FIXME: should be protected
public: // Methods that the derived classes should implement

    /** Fetches more children, and adds them by calling appendChild.
        The amount of children to fetch is up to the implementation.
        After fetching, should call setHasMore.  */
    virtual void fetchMoreChildren()=0;

    virtual void setColumn(int index, const QVariant& data) { Q_UNUSED(index); Q_UNUSED(data); }
    void emitAllChildrenFetched();

protected: // Interface for derived classes

    /** Creates a tree item with the specified data.  
        FIXME: do we actually have to have the model 
        pointer.
     */
    explicit TreeItem(TreeModel* model, TreeItem *parent = nullptr);

    /** Set the data to be shown for the item itself.  */
    void setData(const QVector<QVariant> &data);

    /** Adds a new child and notifies the interested parties.  
        Clears the "hasMore" flag.  */
    void appendChild(TreeItem *child, bool initial = false);

    void insertChild(int position, TreeItem *child, bool initial = false);

    void removeChild(int index);

    void removeSelf();

    void deleteChildren();

    /** Report change in data of this item.  */
    void reportChange();
    void reportChange(int column);

    /** Clears all children.  */
    void clear();

    /** Sets a flag that tells if we have some more children that are
        not fetched yet.  */
    void setHasMore(bool more);

    void setHasMoreInitial(bool more);

    TreeModel* model() { return model_; }

    bool isExpanded() const { return expanded_; }

Q_SIGNALS:
    void expanded();
    void collapsed();
    void allChildrenFetched();

protected: // Backend implementation of Model/View
    friend class TreeModel;

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    virtual QVariant data(int column, int role) const;
    int row() const;
    TreeItem *parent();
    bool hasMore() const { return more_; }
    void setExpanded(bool b);

    virtual void clicked() {}
    virtual QVariant icon(int column) const;

protected:
    QVector<TreeItem*> childItems;
    QVector<QVariant> itemData;
    TreeItem *parentItem;
    TreeModel *model_;
    bool more_;
    TreeItem *ellipsis_;
    bool expanded_;
};

}

#endif
