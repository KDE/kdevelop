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
    : QAbstractItemModel(parent), sourceModel(model), rCount(0)
{
    Q_ASSERT(model);

    connect(sourceModel, SIGNAL(destroyed()), this, SLOT(modelDestroyed()));
    rCount = model->rowCount();
}

QModelIndex QuickOpenFilterModel::index(int row, int column, const QModelIndex &parent) const
{
    if (filterStr.isEmpty())
        return sourceModel->index(row, column, parent);

    if (parent.isValid() || row >= rCount || column != 0)
        return QModelIndex();

    return createIndex(row, column);
}

int QuickOpenFilterModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return rCount;
}

int QuickOpenFilterModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 1;
}

QVariant QuickOpenFilterModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.isValid());

    if (filterStr.isEmpty())
        return sourceModel->data(index, role);

    return sourceModel->data(filteredIdx.at(index.row()), role);
}

QModelIndex QuickOpenFilterModel::parent(const QModelIndex & /*child*/) const
{
    return QModelIndex();
}

void QuickOpenFilterModel::modelDestroyed()
{
    sourceModel = 0;
    qFatal("QuickOpenFilterModel: QuickOpenModel destroued before its filter!");
}

void QuickOpenFilterModel::setFilter(const QString &expression)
{
    filterStr = expression;

    doFiltering();
}

void QuickOpenFilterModel::doFiltering()
{
    filteredIdx.clear();

    QModelIndex idx;
    bool hasHit = false;
    for (int i = 0; i < sourceModel->rowCount(); ++i) {

        idx = sourceModel->index(i, 0);
        Q_ASSERT(idx.isValid());

        if (sourceModel->isTitle(idx)) {
            if (!hasHit && !filteredIdx.isEmpty())
                filteredIdx.pop_back();
            hasHit = false;
            filteredIdx.append(idx);
        } else if (sourceModel->data(idx).toString().contains(filterStr)) {
            hasHit = true;
            filteredIdx.append(idx);
        }
    }
    if (!hasHit && !filteredIdx.isEmpty())
        filteredIdx.pop_back();

    rCount = filteredIdx.count();

    reset();
    // TODO emit signals
}

