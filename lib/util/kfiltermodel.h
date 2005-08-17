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

#ifndef KFILTERMODEL_H
#define KFILTERMODEL_H

#include <qabstractitemmodel.h>
#include <qvector.h>

class KFilterModelPrivate;

class KFilterModel: public QAbstractItemModel
{
    Q_OBJECT
    friend class KFilterModelPrivate;

public:
    explicit KFilterModel(QAbstractItemModel *model, QObject *parent = 0);
    ~KFilterModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex parent(const QModelIndex &child) const;

    QString filter() const;

public slots:
    void setFilter(const QString &expression);

protected:
    virtual bool matches(const QModelIndex &index) const;

private:
    Q_PRIVATE_SLOT(d, void modelChanged())
    Q_PRIVATE_SLOT(d, void modelDestroyed())
    Q_DISABLE_COPY(KFilterModel)

    KFilterModelPrivate *d;
};

#endif

