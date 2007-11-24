/*
 * KDevelop Problem Reporter
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef PROBLEMMODEL_H
#define PROBLEMMODEL_H

#include <QtCore/QAbstractItemModel>

#include <iproblem.h>

class ProblemReporterPart;

class ProblemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ProblemModel(ProblemReporterPart* parent);
    virtual ~ProblemModel();

    enum Columns {
        Error,
        Source,
        File,
        Line,
        Column,
        LastColumn
    };
    
    void addProblem(KDevelop::Problem* problem);
    
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex & index) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    KDevelop::Problem* problemForIndex(const QModelIndex& index) const;
    
private:
    ProblemReporterPart* part() const;

    QList<KDevelop::Problem*> m_problems;
};

#endif // PROBLEMMODEL_H
