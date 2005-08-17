/*
 *  Copyright (C) 2005 Harald Fernengel <harry@kdevelop.org>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include "quickopen_filtermodel.h"

#include <qdebug.h>

QuickOpenFilterModel::QuickOpenFilterModel(QuickOpenModel *model, QObject *parent)
    : QAbstractItemModel(parent), sourceModel(model)
{
    Q_ASSERT(model);

    connect(sourceModel, SIGNAL(destroyed()), this, SLOT(modelDestroyed()));
    connect(sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(modelChanged()));
    connect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(modelChanged()));
    connect(sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(modelChanged()));
    connect(sourceModel, SIGNAL(modelReset()), this, SLOT(modelChanged()));
}

QModelIndex QuickOpenFilterModel::index(int row, int column, const QModelIndex &parent) const
{
    if (filterStr.isEmpty())
        return sourceModel->index(row, column, parent);

    if (column != 0 || filteredIdx.isEmpty())
        return QModelIndex();

    int idx = 0;
    if (parent.isValid()) {
        if (row >= filteredIdx.at(parent.internalId()).childCount)
            return QModelIndex();
        idx += 1;
    }
    idx = nextSiblingIdx(idx, row);
    if (idx == -1)
        return QModelIndex();
    return createIndex(row, column, idx);
}

int QuickOpenFilterModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return topLevelRowCount;
    return filteredIdx.at(parent.internalId()).childCount;
}

int QuickOpenFilterModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

QVariant QuickOpenFilterModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.isValid());

    if (filterStr.isEmpty())
        return sourceModel->data(index, role);

    return sourceModel->data(filteredIdx.at(index.internalId()).index, role);
}

QModelIndex QuickOpenFilterModel::parent(const QModelIndex &child) const
{
    int idx = child.internalId();
    int level = filteredIdx.at(idx).level;
    if (level == 0)
        return QModelIndex();

    while (filteredIdx.at(--idx).level >= level);
    return createIndex(filteredIdx.at(idx).siblingIdx, 0, idx);
}

void QuickOpenFilterModel::modelDestroyed()
{
    sourceModel = 0;
    qFatal("QuickOpenFilterModel: QuickOpenModel destroued while filter-model was active!");
}

void QuickOpenFilterModel::setFilter(const QString &expression)
{
    filterStr = expression;

    doFiltering();
}

int QuickOpenFilterModel::addItems(int level, const QModelIndex &parent)
{
    int siblings = 0;
    for (int i = 0; i < sourceModel->rowCount(parent); ++i) {
        QModelIndex sourceIdx = sourceModel->index(i, 0, parent);
        int currentIdx = -1;
        bool hit = sourceModel->data(sourceIdx).toString().contains(filterStr);
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

void QuickOpenFilterModel::doFiltering()
{
    filteredIdx.clear();
    topLevelRowCount = addItems(0, QModelIndex());
    reset();
}

void QuickOpenFilterModel::modelChanged()
{
    doFiltering();
}

int QuickOpenFilterModel::nextSiblingIdx(int idx, int count) const
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

