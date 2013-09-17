/*
 * This file is part of KDevelop
 * Copyright 2013 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include <QAbstractTableModel>

#include "filter.h"

namespace KDevelop {

class FilterModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit FilterModel(QObject* parent = 0);
    virtual ~FilterModel();

    SerializedFilters filters() const;
    void setFilters(const SerializedFilters& filters);

    void moveFilterUp(int row);
    void moveFilterDown(int row);

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

    virtual Qt::DropActions supportedDropActions() const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
    virtual QMap< int, QVariant > itemData(const QModelIndex& index) const;
    virtual bool setItemData(const QModelIndex& index, const QMap< int, QVariant >& roles);

    enum Columns {
        Pattern,
        Targets,
        Inclusive,
        NUM_COLUMNS
    };

private:
    SerializedFilters m_filters;
    // workaround a strange behavior in Qt when we try to drop after the last item in the list
    bool m_ignoredLastInsert;
};

}

#endif // FILTERMODEL_H
