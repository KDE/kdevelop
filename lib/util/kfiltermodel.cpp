/*
   Copyright (C) 2005 Harald Fernengel <harry@kdevelop.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kfiltermodel.h"

class KFilterModelPrivate
{
public:
    KFilterModelPrivate(KFilterModel *qq, QAbstractItemModel *source)
        : q(qq), sourceModel(source), topLevelRowCount(0) {}

    void doFiltering();
    int nextSiblingIdx(int idx, int count = 1) const;
    int addItems(int level, const QModelIndex &parent);
    void modelDestroyed();
    void modelChanged();
    QModelIndex childIdx(const QModelIndex &index) const;

    KFilterModel *q;
    QAbstractItemModel *sourceModel;
    QString filterStr;
    struct FilterIdx
    {
        QModelIndex index;
        int level;
        int siblingIdx;
        int childCount;
    };
    QVector<FilterIdx> filteredIdx;
    int topLevelRowCount;
};

QModelIndex KFilterModelPrivate::childIdx(const QModelIndex &index) const
{
    QModelIndex idx = filteredIdx.at(index.internalId()).index;
    if (index.column() == 0)
        return idx;
    return sourceModel->index(idx.row(), index.column(), sourceModel->parent(idx));
}

void KFilterModelPrivate::modelDestroyed()
{
    sourceModel = 0;
    qFatal("KFilterModel: QuickOpenModel destroyed while filter-model was active!");
}

void KFilterModelPrivate::modelChanged()
{
    doFiltering();
}

int KFilterModelPrivate::nextSiblingIdx(int idx, int count) const
{
    Q_ASSERT(idx < filteredIdx.count());

    if (!count)
        return idx;

    int level = filteredIdx.at(idx).level;
    while (++idx < filteredIdx.count()) {
        int cLevel = filteredIdx.at(idx).level;
        if (cLevel < level)
            return -1;
        else if (cLevel == level && --count == 0)
            return idx;
    }
    return -1;
}

int KFilterModelPrivate::addItems(int level, const QModelIndex &parent)
{
    int siblings = 0;
    for (int i = 0; i < sourceModel->rowCount(parent); ++i) {
        QModelIndex sourceIdx = sourceModel->index(i, 0, parent);
        int currentIdx = -1;
        bool hit = q->matches(sourceIdx);
        if (hit || sourceModel->hasChildren(sourceIdx)) {
            FilterIdx idx;
            idx.index = sourceIdx;
            idx.level = level;
            idx.siblingIdx = siblings++;
            filteredIdx.append(idx);
            currentIdx = filteredIdx.count() - 1;
        }
        int childCount = 0;
        if (sourceModel->hasChildren(sourceIdx))
            childCount = addItems(level + 1, sourceIdx);
        if (currentIdx != -1) {
            if (!childCount && !hit) {
                filteredIdx.pop_back();
                --siblings;
            } else {
                filteredIdx[currentIdx].childCount = childCount;
            }
        }
    }
    return siblings;
}

void KFilterModelPrivate::doFiltering()
{
    filteredIdx.clear();
    topLevelRowCount = addItems(0, QModelIndex());
    q->reset();
}

KFilterModel::KFilterModel(QAbstractItemModel *model, QObject *parent)
    : QAbstractItemModel(parent), d(new KFilterModelPrivate(this, model))
{
    Q_ASSERT(model);

    connect(model, SIGNAL(destroyed()), this, SLOT(modelDestroyed()));
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(modelChanged()));
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(modelChanged()));
    connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(modelChanged()));
    connect(model, SIGNAL(modelReset()), this, SLOT(modelChanged()));

    d->doFiltering();
}

KFilterModel::~KFilterModel()
{
    delete d;
}

QModelIndex KFilterModel::index(int row, int column, const QModelIndex &parent) const
{
#if 0
    if (d->filterStr.isEmpty())
        return d->sourceModel->index(row, column, parent);
#endif

    if (column != 0 || d->filteredIdx.isEmpty())
        return QModelIndex();

    int idx = 0;
    if (parent.isValid()) {
        if (row >= d->filteredIdx.at(parent.internalId()).childCount)
            return QModelIndex();
        idx += 1;
    }
    idx = d->nextSiblingIdx(idx, row);
    if (idx == -1)
        return QModelIndex();
    return createIndex(row, column, idx);
}

int KFilterModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return d->topLevelRowCount;
    return d->filteredIdx.at(parent.internalId()).childCount;
}

int KFilterModel::columnCount(const QModelIndex & parent) const
{
    QModelIndex idx;
    if (parent.isValid())
        idx = d->filteredIdx.at(parent.internalId()).index;
    return d->sourceModel->columnCount(idx);
}

QVariant KFilterModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.isValid());

#if 0
    if (d->filterStr.isEmpty())
        return d->sourceModel->data(index, role);
#endif

    return d->sourceModel->data(d->childIdx(index), role);
}

QModelIndex KFilterModel::parent(const QModelIndex &child) const
{
    int idx = child.internalId();
    int level = d->filteredIdx.at(idx).level;
    if (level == 0)
        return QModelIndex();

    while (d->filteredIdx.at(--idx).level >= level);
    return createIndex(d->filteredIdx.at(idx).siblingIdx, child.column(), idx);
}

void KFilterModel::setFilter(const QString &expression)
{
    d->filterStr = expression;

    d->doFiltering();
}

/**
 * Matches an index in the model. The default implementation checks whether the item contains
 * the string set by setFilter().
 *
 * @param index The index to match
 * @return true if the item matches, otherwise false
*/
bool KFilterModel::matches(const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    return d->sourceModel->data(index).toString().contains(d->filterStr);
}

QString KFilterModel::filter() const
{
    return d->filterStr;
}

QVariant KFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return d->sourceModel->headerData(section, orientation, role);
}

Qt::ItemFlags KFilterModel::flags(const QModelIndex &index) const
{
    return d->sourceModel->flags(d->childIdx(index));
}

#include "kfiltermodel.moc"
