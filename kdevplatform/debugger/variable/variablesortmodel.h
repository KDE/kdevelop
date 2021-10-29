/*
    SPDX-FileCopyrightText: 2016 Mikhail Ivchenko <ematirov@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VARIABLESORTMODEL_H
#define KDEVPLATFORM_VARIABLESORTMODEL_H

#include <QCollator>
#include <QSortFilterProxyModel>

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
