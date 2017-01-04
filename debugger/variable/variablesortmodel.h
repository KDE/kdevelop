/*
 * KDevelop Debugger Support
 *
 * Copyright 2016 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
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

#ifndef KDEVPLATFORM_VARIABLESORTMODEL_H
#define KDEVPLATFORM_VARIABLESORTMODEL_H

#include <QCollator>
#include <QSortFilterProxyModel>

class QModelIndex;

namespace KDevelop
{

class VariableSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit VariableSortProxyModel(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
private:
    QCollator m_collator;
};

}

#endif
