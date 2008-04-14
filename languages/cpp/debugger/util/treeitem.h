
#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QTimer>
#include <QVector>

#include <iostream>

namespace GDBDebugger {

class TreeModel;

class TreeItem: public QObject
{
    Q_OBJECT
public:
    virtual ~TreeItem();

// FIXME: should be protected
public: // Methods that the derived classes should implement

    /** Fetches more children, and adds them by calling appendChild.
        The amount of children to fetch is up to the implementation.
        After fetching, should call setHasMore.  */
    virtual void fetchMoreChildren() = 0;
       
protected: // Interface for derived classes

    /** Creates a tree item with the specified data.  
        FIXME: do we actually have to have the model 
        pointer.
     */
    TreeItem(TreeModel* model, TreeItem *parent = 0);

    /** Set the data to be shown for the item itself.  */
    void setData(const QVector<QString> &data);

    /** Adds a new child and notifies the interested parties.  
        Clears the "hasMore" flag.  */
    void appendChild(TreeItem *child);

    /** Clears all children.  */
    void clear();

    /** Sets a flag that tells if we have some more children that are
        not fetched yet.  */
    void setHasMore(bool more);

    void setHasMoreInitial(bool more);

    TreeModel* model() { return model_; }

protected: // Backend implementation of Model/View
    friend class TreeModel;

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parent();
    bool hasMore() const { return more_; }

    virtual void clicked() {}

protected:
    QList<TreeItem*> childItems;
    QVector<QVariant> itemData;
    TreeItem *parentItem;
    TreeModel *model_;
    bool more_;
    TreeItem *ellipsis_;
};

}

#endif
