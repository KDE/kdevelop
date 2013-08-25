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

    Filters filters() const;
    void setFilters(const Filters& filters);

    void addFilter();
    void removeFilter(int row);
    void moveFilterUp(int row);
    void moveFilterDown(int row);

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    enum Columns {
        Pattern,
        Targets,
        MatchOn,
        Inclusive,
        NUM_COLUMNS
    };

private:
    Filters m_filters;
};

}

#endif // FILTERMODEL_H
