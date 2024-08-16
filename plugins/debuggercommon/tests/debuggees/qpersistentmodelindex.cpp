/*
    SPDX-FileCopyrightText: 2024 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QPersistentModelIndex>
#include <QStandardItem>
#include <QStandardItemModel>

int main()
{
    QPersistentModelIndex i;

    QStandardItemModel model(2, 2);
    i = model.index(1, 0);
    i = model.index(1, 1);

    auto* const item = new QStandardItem;
    model.setItem(0, 1, item);
    item->appendRow(new QStandardItem);
    i = model.index(0, 0, item->index());

    return 0;
}
