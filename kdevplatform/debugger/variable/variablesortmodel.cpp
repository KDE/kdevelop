/*
    SPDX-FileCopyrightText: 2016 Mikhail Ivchenko <ematirov@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "variablesortmodel.h"

namespace KDevelop
{

VariableSortProxyModel::VariableSortProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    m_collator.setNumericMode(true);
    m_collator.setCaseSensitivity(Qt::CaseInsensitive);
}

bool VariableSortProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const QString leftString = sourceModel()->data(left).toString();
    const QString rightString = sourceModel()->data(right).toString();
    int result = m_collator.compare(leftString, rightString);
    return result < 0;
}

}
