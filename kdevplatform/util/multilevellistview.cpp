/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "multilevellistview.h"

#include <QHBoxLayout>
#include <QTreeView>
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

    QVariant header(QAbstractItemModel* model, int section, Qt::Orientation orientation, int role) const
    {
        if (model && section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            return m_label;
        } else {
            return QVariant();
        }
    }

protected:
    QString m_label;
};

/**
 * The left-most view's model which only contains the root nodes of the source model.
 */
class RootProxyModel : public QSortFilterProxyModel
    , public LabeledProxy
{
    Q_OBJECT

public:
    explicit RootProxyModel(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
    }
    bool filterAcceptsRow(int /*source_row*/, const QModelIndex& source_parent) const override
    {
        return !source_parent.isValid();
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        return header(sourceModel(), section, orientation, role);
    }
};

/**
 * A class that automatically updates its contents based on the selection in another view.
 */
class SubTreeProxyModel : public KSelectionProxyModel
    , public LabeledProxy
{
    Q_OBJECT

public:
    explicit SubTreeProxyModel(QItemSelectionModel* selectionModel, QObject* parent = nullptr)
        : KSelectionProxyModel(selectionModel, parent)
    {}
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        return header(sourceModel(), section, orientation, role);
    }
    Qt::ItemFlags flags(const QModelIndex& index) const override
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
    explicit MultiLevelListViewPrivate(MultiLevelListView* view);
    ~MultiLevelListViewPrivate();

    void viewSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void lastViewsContentsChanged();
    void ensureViewSelected(QTreeView* view);

    /**
     * @param index index in any of our proxy models
     * @return an index in the source model
     */
    QModelIndex mapToSource(QModelIndex index) const;
    /**
     * @param index an index in the source model
     * @return an index in the view's model at level @p level
     */
    QModelIndex mapFromSource(QModelIndex index, int level) const;

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
    , model(nullptr)
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
    auto* proxy = qobject_cast<QAbstractProxyModel*>(
        const_cast<QAbstractItemModel*>(current.model()));
    Q_ASSERT(proxy);

    // what level is this proxy in
    int level = -1;
    for (int i = 0; i < levels; ++i) {
        if (views.at(i)->model() == proxy) {
            level = i;
            break;
        }
    }

    Q_ASSERT(level >= 0 && level < levels);

    if (level + 1 == levels) {
        // right-most view
        if (proxy->hasIndex(0, 0, current)) {
            // select the first leaf node for this view
            QModelIndex idx = current;
            QModelIndex child = proxy->index(0, 0, idx);
            while (child.isValid()) {
                idx = child;
                child = proxy->index(0, 0, idx);
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

QModelIndex MultiLevelListViewPrivate::mapToSource(QModelIndex index) const
{
    if (!index.isValid()) {
        return index;
    }

    while (index.model() != model) {
        auto* proxy = qobject_cast<QAbstractProxyModel*>(
            const_cast<QAbstractItemModel*>(index.model()));
        Q_ASSERT(proxy);
        index = proxy->mapToSource(index);
        Q_ASSERT(index.isValid());
    }
    return index;
}

QModelIndex MultiLevelListViewPrivate::mapFromSource(QModelIndex index, int level) const
{
    if (!index.isValid()) {
        return index;
    }

    Q_ASSERT(index.model() == model);

    auto* proxy = qobject_cast<QAbstractProxyModel*>(views.at(level)->model());
    Q_ASSERT(proxy);

    // find all proxies between the source and our view
    QVector<QAbstractProxyModel*> proxies;
    proxies << proxy;
    while (true) {
        auto* child = qobject_cast<QAbstractProxyModel*>(proxy->sourceModel());
        if (child) {
            proxy = child;
            proxies << proxy;
        } else {
            Q_ASSERT(proxy->sourceModel() == model);
            break;
        }
    }
    // iterate in reverse order to find the view's index
    for (int i = proxies.size() - 1; i >= 0; --i) {
        proxy = proxies.at(i);
        index = proxy->mapFromSource(index);
        Q_ASSERT(index.isValid());
    }

    return index;
}

MultiLevelListView::MultiLevelListView(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , d_ptr(new MultiLevelListViewPrivate(this))
{
    setLayout(new QHBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);

    qRegisterMetaType<QTreeView*>("QTreeView*");
}

MultiLevelListView::~MultiLevelListView() = default;

int MultiLevelListView::levels() const
{
    Q_D(const MultiLevelListView);

    return d->levels;
}

void MultiLevelListView::setLevels(int levels)
{
    Q_D(MultiLevelListView);

    qDeleteAll(d->views);
    qDeleteAll(d->proxies);
    qDeleteAll(d->layouts);
    d->views.clear();
    d->proxies.clear();
    d->layouts.clear();

    d->levels = levels;
    d->views.reserve(levels);
    d->proxies.reserve(levels);
    d->layouts.reserve(levels);

    QTreeView* previousView = nullptr;
    for (int i = 0; i < d->levels; ++i) {
        auto* levelLayout = new QVBoxLayout();

        auto* view = new QTreeView(this);
        view->setContentsMargins(0, 0, 0, 0);
        // only the right-most view is decorated
        view->setRootIsDecorated(i + 1 == d->levels);
        view->setHeaderHidden(false);
        view->setSelectionMode(QAbstractItemView::SingleSelection);

        if (!previousView) {
            // the root, i.e. left-most view
            auto* root = new RootProxyModel(this);
            root->setDynamicSortFilter(true);
            d->proxies << root;
            root->setSourceModel(d->model);
            view->setModel(root);
        } else {
            auto* subTreeProxy = new SubTreeProxyModel(previousView->selectionModel(), this);
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
            auto* sortProxy = new QSortFilterProxyModel(subTreeProxy);
            sortProxy->setSourceModel(subTreeProxy);
            sortProxy->setDynamicSortFilter(true);
            view->setModel(sortProxy);
        }

        // view->setModel creates the selection model
        connect(view->selectionModel(), &QItemSelectionModel::currentChanged,
                this, [this](const QModelIndex& current, const QModelIndex& previous) {
            Q_D(MultiLevelListView);
            d->viewSelectionChanged(current, previous);
        });

        if (i + 1 == d->levels) {
            connect(view->model(), &QAbstractItemModel::rowsInserted,
                    this, [this] {
                Q_D(MultiLevelListView);
                d->lastViewsContentsChanged();
            });
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
    Q_D(const MultiLevelListView);

    return d->model;
}

void MultiLevelListView::setModel(QAbstractItemModel* model)
{
    Q_D(MultiLevelListView);

    d->model = model;

    for (LabeledProxy* proxy : std::as_const(d->proxies)) {
        dynamic_cast<QAbstractProxyModel*>(proxy)->setSourceModel(model);
    }

    if (model && !d->views.isEmpty()) {
        d->views.first()->setCurrentIndex(d->views.first()->model()->index(0, 0));
    }
}

QTreeView* MultiLevelListView::viewForLevel(int level) const
{
    Q_D(const MultiLevelListView);

    return d->views.value(level);
}

void MultiLevelListView::addWidget(int level, QWidget* widget)
{
    Q_D(MultiLevelListView);

    Q_ASSERT(level < d->levels);
    d->layouts[level]->addWidget(widget);
}

QModelIndex MultiLevelListView::currentIndex() const
{
    Q_D(const MultiLevelListView);

    return d->mapToSource(d->views.last()->currentIndex());
}

void MultiLevelListView::setCurrentIndex(const QModelIndex& index)
{
    Q_D(MultiLevelListView);

    // incoming index is for the original model
    Q_ASSERT(!index.isValid() || index.model() == d->model);

    // Notify about the change to a possibly invalid index in case the code below fails to select
    // a valid index in the rightmost view and consequently does not emit currentIndexChanged().
    emit currentIndexChanged(index, currentIndex());

    QModelIndex idx(index);
    QVector<QModelIndex> indexes;

    while (idx.isValid()) {
        indexes.prepend(idx);
        idx = idx.parent();
    }

    for (int i = 0; i < d->levels; ++i) {
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
}

void MultiLevelListView::setRootIndex(const QModelIndex& index)
{
    Q_D(MultiLevelListView);

    Q_ASSERT(!index.isValid() || index.model() == d->model);
    d->views.first()->setRootIndex(index);
}

void MultiLevelListView::setHeaderLabels(const QStringList& labels)
{
    Q_D(MultiLevelListView);

    int n = qMin(d->levels, labels.size());
    for (int i = 0; i < n; ++i) {
        d->proxies.at(i)->setLabel(labels[i]);
    }
}

static
KSelectionProxyModel::FilterBehavior toSelectionProxyModelFilterBehavior(MultiLevelListView::LastLevelViewMode mode)
{
    switch (mode) {
    case MultiLevelListView::SubTrees:
        return KSelectionProxyModel::SubTreesWithoutRoots;
    case MultiLevelListView::DirectChildren:
        return KSelectionProxyModel::ChildrenOfExactSelection;
    }
    Q_UNREACHABLE();
}

void MultiLevelListView::setLastLevelViewMode(LastLevelViewMode mode)
{
    Q_D(MultiLevelListView);

    if (d->proxies.isEmpty()) {
        return;
    }
    const auto filterBehavior = toSelectionProxyModelFilterBehavior(mode);
    dynamic_cast<KSelectionProxyModel*>(d->proxies.last())->setFilterBehavior(filterBehavior);
}

#include "multilevellistview.moc"
#include "moc_multilevellistview.cpp"
