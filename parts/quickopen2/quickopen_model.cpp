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

#include "quickopen_model.h"

#include <qdebug.h>

QuickOpenModel::QuickOpenModel(QObject *parent)
    : QAbstractItemModel(parent), rCount(0)
{

}

QModelIndex QuickOpenModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || row >= rCount || column != 0)
        return QModelIndex();

    // figure out in which child model the row is so lookups are faster
    int i;
    for (i = 0; i < cModels.count(); ++i) {
        const CModel &cm = cModels.at(i);
        if (i == cModels.count() - 1 || (cm.rowCount && row > cm.rowIndex))
            break;
    }

    qDebug() << "requested index for" << row << column << "i is" << i;

    return createIndex(row, column, i);
}

int QuickOpenModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return rCount;
}

int QuickOpenModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 1;
}

QVariant QuickOpenModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    Q_ASSERT(index.internalId() < cModels.count());


    const CModel &cm = cModels.at(int(index.internalId()));
    int childRow = index.row() - cm.rowIndex - 1; // row in our child model
    qDebug() << "wants data for" << index << "childRow is" << childRow;
    if (childRow == -1)
        return role == Qt::DisplayRole ? QVariant(cm.title) : QVariant();
    qDebug() << "data: " << cm.model->data(cm.model->index(childRow, index.column()), role);

    return cm.model->data(cm.model->index(childRow, index.column()), role);
}

QModelIndex QuickOpenModel::parent(const QModelIndex & /*child*/) const
{
    return QModelIndex();
}

void QuickOpenModel::addChildModel(QAbstractItemModel *childModel, const QString &title)
{
    Q_ASSERT(childModel);

    connect(childModel, SIGNAL(destroyed(QObject*)), this, SLOT(removeModelPrivate(QObject*)));
    int childCount = childModel->rowCount();
    CModel cm;
    cm.model = childModel;
    cm.rowCount = childCount;
    cm.rowIndex = 0;
    cm.title = title;
    if (!cModels.isEmpty()) {
        const CModel &lastCm = cModels.last();
        cm.rowIndex = lastCm.rowIndex + lastCm.rowCount;
        if (lastCm.rowCount)
            cm.rowIndex += 1;
    }
    cModels.append(cm);
    if (childCount)
        rCount += childCount + 1;

    qDebug() << "Added model with rowCount" << childCount << "rCount is" << rCount << "rowIndex is" << cm.rowIndex;
    
    // TODO - emit signals
}

void QuickOpenModel::removeModel(QAbstractItemModel *childModel)
{
    for (int i = 0; i < cModels.count(); ++i) {
        if (cModels.at(i).model == childModel) {
            cModels.remove(i);
            break;
        }
    }
    refresh();
    // TODO - emit signals
}

void QuickOpenModel::removeModelPrivate(QObject *childModel)
{
    Q_ASSERT(qobject_cast<QAbstractItemModel *>(childModel));
    removeModel(static_cast<QAbstractItemModel *>(childModel));
}

void QuickOpenModel::refresh()
{
    rCount = 0;
    for (int i = 0; i < cModels.count(); ++i) {
        cModels[i].rowIndex = rCount;
        int childCount = cModels.at(i).rowCount;
        if (childCount)
            rCount += childCount + 1;
    }
}

QList<QAbstractItemModel *> QuickOpenModel::childModels() const
{
    QList<QAbstractItemModel *> list;
    for (int i = 0; i < cModels.count(); ++i)
        list << cModels.at(i).model;
    return list;
}

