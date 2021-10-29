/*
    SPDX-FileCopyrightText: 2012-2013 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <ghprovidermodel.h>


namespace gh
{

ProviderItem::ProviderItem(const Response &r)
    : QStandardItem(r.name), m_data(r)
{
    if (r.kind == Private)
        setIcon(QIcon::fromTheme(QStringLiteral("github-private")));
    else if (r.kind == Fork)
        setIcon(QIcon::fromTheme(QStringLiteral("github-forked")));
    else
        setIcon(QIcon::fromTheme(QStringLiteral("github-repo")));
}

QVariant ProviderItem::data(int role) const
{
    switch(role) {
        case ProviderModel::VcsLocationRole:
            return QVariant(m_data.url);
        default:
            return QStandardItem::data(role);
    }
}

ProviderModel::ProviderModel(QObject *parent)
    : QStandardItemModel(parent)
{
    /* There's nothing to do here */
}

} // End of namespace gh
