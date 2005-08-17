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

#include <qabstractitemmodel.h>
#include <qvector.h>

#ifndef QUICKOPEN_MODEL_H
#define QUICKOPEN_MODEL_H

class QuickOpenModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit QuickOpenModel(QObject *parent = 0);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex parent(const QModelIndex &child) const;

    void addChildModel(QAbstractItemModel *childModel, const QString &title);
    QList<QAbstractItemModel *> childModels() const;
    QString modelTitle(QAbstractItemModel *childModel) const;
    bool isTitle(const QModelIndex &index) const;

public slots:
    void removeModel(QAbstractItemModel *childModel);

private slots:
    void removeModelPrivate(QObject *childModel);
    void childModelReset();
    void childModelRowsAboutToBeInserted(const QModelIndex &parent, int first, int last);
    void childModelRowsInserted(const QModelIndex &parent, int first, int last);
    void childModelDataChanged(const QModelIndex &first, const QModelIndex &last);
    void childModelRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void childModelRowsRemoved(const QModelIndex &parent, int first, int last);

private:
    void refresh();
    QModelIndex convertChildModelIndex(const QModelIndex &childModelIndex) const;
    int convertChildModelRow(const QAbstractItemModel *model, int row, int *internalId = 0) const;

    struct CModel
    {
        QString title;
        QAbstractItemModel *model;
        int rowCount;
        int rowIndex;
    };
    QVector<CModel> cModels;

    int rCount;
};

#endif

