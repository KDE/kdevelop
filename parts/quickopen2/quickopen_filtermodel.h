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

#ifndef QUICKOPEN_FILTERMODEL_H
#define QUICKOPEN_FILTERMODEL_H

#include "quickopen_model.h"

class QuickOpenFilterModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit QuickOpenFilterModel(QuickOpenModel *model, QObject *parent = 0);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex parent(const QModelIndex &child) const;

    inline QString filter() const { return filterStr; }

public slots:
    void setFilter(const QString &expression);

private slots:
    void modelDestroyed();

private:
    void doFiltering();

    QuickOpenModel *sourceModel;
    int rCount;
    QString filterStr;
    QVector<QModelIndex> filteredIdx;
};

#endif

