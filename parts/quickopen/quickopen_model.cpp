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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "quickopen_model.h"


#include <QtGui/QtGui>
#include <qdebug.h>

QuickOpenModel::QuickOpenModel(QObject *parent)
    : QAbstractItemModel(parent)
{

}

QModelIndex QuickOpenModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column != 0)
        return QModelIndex();

    if (!parent.isValid()) {
        if (row >= cModels.count() || row < 0)
            return QModelIndex();
        return createIndex(row, 0);
    } else if (parent.model() == this) {
        return cModels.at(parent.row()).model->index(row, column);
    } else {
        return parent.model()->index(row, column, parent);
    }
}

int QuickOpenModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return cModels.count();
    else if (parent.model() == this)
        return cModels.at(parent.row()).model->rowCount();
    else
        return parent.model()->rowCount(parent);
}

bool QuickOpenModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return !cModels.isEmpty();
    if (parent.model() == this)
        return cModels.at(parent.row()).model->hasChildren();
    return parent.model()->hasChildren(parent);
}

int QuickOpenModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant QuickOpenModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.isValid());

    if (index.model() == this) {
        return role == Qt::DisplayRole ? QVariant(cModels.at(index.row()).title) : QVariant();
    }
    return index.model()->data(index, role);
}

bool QuickOpenModel::isTitle(const QModelIndex &index) const
{
    return index.isValid() && index.model() == this;
}

QModelIndex QuickOpenModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    if (child.model() == this)
        return QModelIndex();

    QModelIndex pidx = child.model()->parent(child);
    if (!pidx.isValid())
        return index(indexOf(child.model()), 0);
    return pidx;
}

int QuickOpenModel::indexOf(const QAbstractItemModel *model) const
{
    for (int i = 0; i < cModels.count(); ++i) {
        if (cModels.at(i).model == model)
            return i;
    }
    return -1;
}

void QuickOpenModel::addChildModel(QAbstractItemModel *childModel, const QString &title)
{
    Q_ASSERT(childModel);

    CModel cm;
    cm.model = childModel;
    cm.title = title;

    beginInsertRows(QModelIndex(), cModels.count(), 1);

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

    cModels.append(cm);
    endInsertRows();
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
            beginRemoveRows(QModelIndex(), i, i);
            cModels.remove(i);
            endRemoveRows();
            break;
        }
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
    return cModels.value(indexOf(childModel)).title;
}

void QuickOpenModel::childModelReset()
{
   reset();
}

void QuickOpenModel::childModelRowsAboutToBeInserted(const QModelIndex &parent, int first, int last)
{
    beginInsertRows(parent, first, last);
}

void QuickOpenModel::childModelRowsInserted(const QModelIndex &parent, int /*first*/, int /*last*/)
{
    endInsertRows();
}

void QuickOpenModel::childModelDataChanged(const QModelIndex &first, const QModelIndex &last)
{
    emit dataChanged(first, last);
}

void QuickOpenModel::childModelRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    beginRemoveRows(parent, first, last);
}

void QuickOpenModel::childModelRowsRemoved(const QModelIndex &parent, int /*first*/, int /*last*/)
{
    endRemoveRows();
}


