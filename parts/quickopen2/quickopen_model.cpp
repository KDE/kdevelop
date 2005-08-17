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


#include <QtGui/QtGui>
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
        if (row < cm.rowIndex)
            break;
    }
    if (i != 0)
        --i;

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
    Q_ASSERT(index.isValid());
    Q_ASSERT(index.internalId() < cModels.count());


    const CModel &cm = cModels.at(int(index.internalId()));
    int childRow = index.row() - cm.rowIndex - 1; // row in our child model
    if (childRow == -1) {
        if (role == Qt::BackgroundColorRole)
            return QColor(Qt::lightGray);
        return role == Qt::DisplayRole ? QVariant(cm.title) : QVariant();
    }

    return cm.model->data(cm.model->index(childRow, index.column()), role);
}

bool QuickOpenModel::isTitle(const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());
    return index.row() - cModels.at(int(index.internalId())).rowIndex == 0;
}

QModelIndex QuickOpenModel::parent(const QModelIndex & /*child*/) const
{
    return QModelIndex();
}

void QuickOpenModel::addChildModel(QAbstractItemModel *childModel, const QString &title)
{
    Q_ASSERT(childModel);

    int childCount = childModel->rowCount();
    if (childCount)
        beginInsertRows(QModelIndex(), rCount, rCount + childCount + 1);

    connect(childModel, SIGNAL(destroyed(QObject*)), this, SLOT(removeModelPrivate(QObject*)));
    connect(childModel, SIGNAL(modelReset()), this, SLOT(childModelReset()));
    connect(childModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), this,
            SLOT(childModelRowsAboutToBeInserted(QModelIndex,int,int)));
    connect(childModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this,
            SLOT(childModelRowsInserted(QModelIndex,int,int)));
    connect(childModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this,
            SLOT(childModelDataChanged(QModelIndex,QModelIndex)));
    connect(childModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this,
            SLOT(childModelRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(childModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this,
            SLOT(childModelRowsRemoved(QModelIndex,int,int)));
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
    if (childCount) {
        rCount += childCount + 1;
        endInsertRows();
    }
}

void QuickOpenModel::removeModel(QAbstractItemModel *childModel)
{
    disconnect(childModel, 0, this, 0);
    removeModelPrivate(childModel);
}

void QuickOpenModel::removeModelPrivate(QObject *childModel)
{
    for (int i = 0; i < cModels.count(); ++i) {
        if (cModels.at(i).model == childModel) {
            cModels.remove(i);
            break;
        }
    }
    refresh();
    reset();
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

QString QuickOpenModel::modelTitle(QAbstractItemModel *childModel) const
{
    for (int i = 0; i < cModels.count(); ++i) {
        if (cModels.at(i).model == childModel)
            return cModels.at(i).title;
    }
    return QString();
}

void QuickOpenModel::childModelReset()
{
    refresh();
    reset();
}

int QuickOpenModel::convertChildModelRow(const QAbstractItemModel *model, int row, int *internalId) const
{
    Q_ASSERT(model);

    int rowIndex = 0;
    int i;
    for (i = 0; i < cModels.count(); ++i) {
        const CModel &cm = cModels.at(i);
        if (cm.model == model) {
            rowIndex = cm.rowIndex;
            if (cm.rowCount)
                rowIndex += 1;
            break;
        }
    }
    if (internalId)
        *internalId = i;
    return row + rowIndex;
}

QModelIndex QuickOpenModel::convertChildModelIndex(const QModelIndex &childModelIndex) const
{
    int internalId = 0;
    int parentRow = convertChildModelRow(childModelIndex.model(), childModelIndex.row(), &internalId);
    return createIndex(parentRow, childModelIndex.column(), internalId);
}

void QuickOpenModel::childModelRowsAboutToBeInserted(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
        return;

    int internalId = 0;
    int rowOffset = convertChildModelRow(qobject_cast<const QAbstractItemModel *>(sender()), 0, &internalId);
    first += rowOffset;
    last += rowOffset;
    if (cModels.at(internalId).rowCount == 0)
        last += 1;
    beginInsertRows(parent, first, last);
}

void QuickOpenModel::childModelRowsInserted(const QModelIndex &parent, int /*first*/, int /*last*/)
{
    if (parent.isValid())
        return;

    endInsertRows();
}

void QuickOpenModel::childModelDataChanged(const QModelIndex &first, const QModelIndex &last)
{
    if (first.parent().isValid() || first.column() != 0)
        return;

    emit dataChanged(convertChildModelIndex(first), convertChildModelIndex(last));
}

void QuickOpenModel::childModelRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
        return;

    int internalId = 0;
    int rowOffset = convertChildModelRow(qobject_cast<const QAbstractItemModel *>(sender()), 0, &internalId);
    first += rowOffset;
    last += rowOffset;

    // remove title as well if the child model doesn't have rows left
    if (last - first == cModels.at(internalId).rowCount)
        first -= 1;
    beginRemoveRows(parent, first, last);
}

void QuickOpenModel::childModelRowsRemoved(const QModelIndex &parent, int /*first*/, int /*last*/)
{
    if (parent.isValid())
        return;

    refresh();
    endRemoveRows();
}

