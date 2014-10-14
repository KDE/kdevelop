/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "multilevellistview.h"

#include <QHBoxLayout>
#include <QTreeView>
#include <QLayout>
#include <QSortFilterProxyModel>

#include <KSelectionProxyModel>

/**
 * Interface to set the label of a model.
 */
class LabeledProxy
{
public:
    virtual ~LabeledProxy()
    {
    }
    void setLabel(const QString& label)
    {
        m_label = label;
    }
protected:
    QString m_label;
};

/**
 * The left-most view's model which only contains the root nodes of the source model.
 */
class RootProxyModel : public QSortFilterProxyModel, public LabeledProxy
{
    Q_OBJECT

public:
    RootProxyModel( QObject* parent = 0 )
    : QSortFilterProxyModel( parent )
    {
    }
    virtual bool filterAcceptsRow( int /*source_row*/, const QModelIndex& source_parent ) const
    {
        return !source_parent.isValid();
    }
    virtual QVariant headerData( int section, Qt::Orientation orientation,
                                 int role = Qt::DisplayRole ) const
    {
        if (sourceModel() && section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            return m_label;
        } else {
            return QVariant();
        }
    }
};

/**
 * A class that automatically updates its contents based on the selection in another view.
 */
class SubTreeProxyModel : public KSelectionProxyModel, public LabeledProxy
{
    Q_OBJECT

public:
    explicit SubTreeProxyModel( QItemSelectionModel* selectionModel, QObject* parent = 0 )
    : KSelectionProxyModel( selectionModel, parent )
    {}
    virtual QVariant headerData( int section, Qt::Orientation orientation,
                                 int role = Qt::DisplayRole ) const
    {
        if (sourceModel() && section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            return m_label;
        } else {
            return QVariant();
        }
    }
    virtual Qt::ItemFlags flags(const QModelIndex& index) const
    {
        Qt::ItemFlags ret = KSelectionProxyModel::flags(index);
        if (filterBehavior() == KSelectionProxyModel::SubTreesWithoutRoots && hasChildren(index)) {
            // we want to select child items
            ret &= ~Qt::ItemIsSelectable;
        }
        return ret;
    }
};

using namespace KDevelop;

class KDevelop::MultiLevelListViewPrivate
{
public:
    MultiLevelListViewPrivate(MultiLevelListView* view);
    ~MultiLevelListViewPrivate();

    void viewSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void lastViewsContentsChanged();
    void ensureViewSelected(QTreeView* view);

    /**
     * @param index index in any of our proxy models
     * @return an index in the source model
     */
    QModelIndex mapToSource(QModelIndex index);
    /**
     * @param index an index in the source model
     * @return an index in the view's model at level @p level
     */
    QModelIndex mapFromSource(QModelIndex index, int level);

    MultiLevelListView* view;

    int levels;
    QList<QTreeView*> views;
    QList<LabeledProxy*> proxies;
    QList<QVBoxLayout*> layouts;

    QAbstractItemModel* model;
};

MultiLevelListViewPrivate::MultiLevelListViewPrivate(MultiLevelListView* view_)
: view(view_)
, levels(0)
, model(0)
{
}

MultiLevelListViewPrivate::~MultiLevelListViewPrivate()
{
}

void MultiLevelListViewPrivate::viewSelectionChanged(const QModelIndex& current,
                                                     const QModelIndex& previous)
{
    if (!current.isValid()) {
        // ignore, as we should always have some kind of selection
        return;
    }

    // figure out which proxy this signal belongs to
    QAbstractProxyModel* proxy = qobject_cast<QAbstractProxyModel*>(
        const_cast<QAbstractItemModel*>(current.model()));
    Q_ASSERT(proxy);

    // what level is this proxy in
    int level = -1;
    for(int i = 0; i < levels; ++i) {
        if (views.at(i)->model() == proxy) {
            level = i;
            break;
        }
    }
    Q_ASSERT(level >= 0 && level < levels);

    if (level + 1 == levels) {
        // right-most view
        if (current.child(0, 0).isValid()) {
            // select the first leaf node for this view
            QModelIndex idx = current;
            QModelIndex child = idx.child(0, 0);
            while(child.isValid()) {
                idx = child;
                child = idx.child(0, 0);
            }
            views.last()->setCurrentIndex(idx);
            return;
        }
        // signal that our actual selection has changed
        emit view->currentIndexChanged(mapToSource(current), mapToSource(previous));
    } else {
        // some leftish view
        // ensure the next view's first item is selected
        QTreeView* treeView = views.at(level + 1);

        // we need to delay the call, because at this point the child view
        // will still have its old data which is going to be invalidated
        // right after this method exits
        // be we must not set the index to 0,0 here directly, since e.g.
        // MultiLevelListView::setCurrentIndex might have been used, which
        // sets a proper index already.
        QMetaObject::invokeMethod(view, "ensureViewSelected", Qt::QueuedConnection,
                                  Q_ARG(QTreeView*, treeView));
    }
}

void MultiLevelListViewPrivate::lastViewsContentsChanged()
{
    views.last()->expandAll();
}

void MultiLevelListViewPrivate::ensureViewSelected(QTreeView* view)
{
    if (!view->currentIndex().isValid()) {
        view->setCurrentIndex(view->model()->index(0, 0));
    }
}

QModelIndex MultiLevelListViewPrivate::mapToSource(QModelIndex index)
{
    if (!index.isValid()) {
        return index;
    }

    while(index.model() != model) {
        QAbstractProxyModel* proxy = qobject_cast<QAbstractProxyModel*>(
            const_cast<QAbstractItemModel*>(index.model()));
        Q_ASSERT(proxy);
        index = proxy->mapToSource(index);
        Q_ASSERT(index.isValid());
    }
    return index;
}

QModelIndex MultiLevelListViewPrivate::mapFromSource(QModelIndex index, int level)
{
    if (!index.isValid()) {
        return index;
    }

    Q_ASSERT(index.model() == model);

    QAbstractProxyModel* proxy = qobject_cast<QAbstractProxyModel*>(views.at(level)->model());
    Q_ASSERT(proxy);

    // find all proxies between the source and our view
    QVector<QAbstractProxyModel*> proxies;
    proxies << proxy;
    forever {
        QAbstractProxyModel* child = qobject_cast<QAbstractProxyModel*>(proxy->sourceModel());
        if (child) {
            proxy = child;
            proxies << proxy;
        } else {
            Q_ASSERT(proxy->sourceModel() == model);
            break;
        }
    }
    // iterate in reverse order to find the view's index
    for(int i = proxies.size() - 1; i >= 0; --i) {
        proxy = proxies.at(i);
        index = proxy->mapFromSource(index);
        Q_ASSERT(index.isValid());
    }
    return index;
}

MultiLevelListView::MultiLevelListView(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, d(new MultiLevelListViewPrivate(this))
{
    delete layout();
    setLayout(new QHBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);

    qRegisterMetaType<QTreeView*>("QTreeView*");
}

MultiLevelListView::~MultiLevelListView()
{
    delete d;
}

int MultiLevelListView::levels() const
{
    return d->levels;
}

void MultiLevelListView::setLevels(int levels)
{
    qDeleteAll(d->views);
    qDeleteAll(d->proxies);
    qDeleteAll(d->layouts);
    d->views.clear();
    d->proxies.clear();
    d->layouts.clear();

    d->levels = levels;

    QTreeView* previousView = 0;
    for (int i = 0; i < d->levels; ++i)
    {
        QVBoxLayout* levelLayout = new QVBoxLayout();

        QTreeView* view = new QTreeView(this);
        view->setContentsMargins(0, 0, 0, 0);
        // only the right-most view is decorated
        view->setRootIsDecorated(i + 1 == d->levels);
        view->setHeaderHidden(false);
        view->setSelectionMode(QAbstractItemView::SingleSelection);

        if (!previousView) {
            // the root, i.e. left-most view
            RootProxyModel* root = new RootProxyModel(this);
            root->setDynamicSortFilter(true);
            d->proxies << root;
            root->setSourceModel(d->model);
            view->setModel(root);
        } else {
            SubTreeProxyModel* subTreeProxy = new SubTreeProxyModel(previousView->selectionModel(), this);
            if (i + 1 < d->levels) {
                // middel views only shows children of selection
                subTreeProxy->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);
            } else {
                // right-most view shows the rest
                subTreeProxy->setFilterBehavior(KSelectionProxyModel::SubTreesWithoutRoots);
            }
            d->proxies << subTreeProxy;
            subTreeProxy->setSourceModel(d->model);
            // sorting requires another proxy in-between
            QSortFilterProxyModel* sortProxy = new QSortFilterProxyModel(subTreeProxy);
            sortProxy->setSourceModel(subTreeProxy);
            sortProxy->setDynamicSortFilter(true);
            view->setModel(sortProxy);
        }

        // view->setModel creates the selection model
        connect(view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(viewSelectionChanged(QModelIndex,QModelIndex)));

        if (i + 1 == d->levels) {
            connect(view->model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
                    SLOT(lastViewsContentsChanged()));
        }

        view->setSortingEnabled(true);
        view->sortByColumn(0, Qt::AscendingOrder);

        levelLayout->addWidget(view);
        layout()->addItem(levelLayout);

        d->layouts << levelLayout;
        d->views << view;

        previousView = view;
    }

    setModel(d->model);
}

QAbstractItemModel* MultiLevelListView::model() const
{
    return d->model;
}

void MultiLevelListView::setModel(QAbstractItemModel* model)
{
    d->model = model;

    foreach (LabeledProxy* proxy, d->proxies)
    {
        dynamic_cast<QAbstractProxyModel*>(proxy)->setSourceModel(model);
    }

    if (model && !d->views.isEmpty())
    {
        d->views.first()->setCurrentIndex(d->views.first()->model()->index(0, 0));
    }
}

QTreeView* MultiLevelListView::viewForLevel( int level ) const
{
    return d->views[level];
}

void MultiLevelListView::addWidget(int level, QWidget* widget)
{
    Q_ASSERT(level < d->levels);
    d->layouts[level]->addWidget(widget);
}

QModelIndex MultiLevelListView::currentIndex() const
{
    return d->mapToSource(d->views.last()->currentIndex());
}

void MultiLevelListView::setCurrentIndex(const QModelIndex& index)
{
    // incoming index is for the original model
    Q_ASSERT(!index.isValid() || index.model() == d->model);

    const QModelIndex previous = currentIndex();

    QModelIndex idx(index);
    QModelIndexList indexes;

    while (idx.isValid())
    {
        indexes.prepend(idx);
        idx = idx.parent();
    }

    for (int i = 0; i < d->levels; ++i)
    {
        QTreeView* view = d->views.at(i);
        if (indexes.size() <= i) {
            // select first item by default
            view->setCurrentIndex(view->model()->index(0, 0));
            continue;
        }

        QModelIndex index;
        if (i + 1 == d->levels) {
            // select the very last index in the list (i.e. might be deep down in the actual tree)
            index = indexes.last();
        } else {
            // select the first index for that level
            index = indexes.at(i);
        }
        view->setCurrentIndex(d->mapFromSource(index, i));
    }

    emit currentIndexChanged(index, previous);
}

void MultiLevelListView::setRootIndex(const QModelIndex& index)
{
    Q_ASSERT(!index.isValid() || index.model() == d->model);
    d->views.first()->setRootIndex(index);
}

void MultiLevelListView::setHeaderLabels(const QStringList& labels)
{
    int n = qMin(d->levels, labels.size());
    for (int i = 0; i < n; ++i)
    {
        d->proxies.at(i)->setLabel(labels[i]);
    }
}

void MultiLevelListView::setLastModelsFilterBehavior(KSelectionProxyModel::FilterBehavior filter)
{
    if (d->proxies.isEmpty()) {
        return;
    }
    dynamic_cast<KSelectionProxyModel*>(d->proxies.last())->setFilterBehavior(filter);
}


#include "multilevellistview.moc"
#include "moc_multilevellistview.cpp"
