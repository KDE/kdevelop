/*
 *   KDevelop outline view
 *   Copyright 2010, 2015 Alex Richardson <alex.richardson@gmx.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#pragma once

#include <QSortFilterProxyModel>

class OutlineProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit OutlineProxyModel(QObject* parent = 0);
    virtual ~OutlineProxyModel();
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
private:
    Q_DISABLE_COPY(OutlineProxyModel)
};
