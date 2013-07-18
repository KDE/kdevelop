/* This file is part of KDevelop
 *
 * Copyright (C) 2012-2013 Miquel Sabaté <mikisabate@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <KIcon>
#include <ghprovidermodel.h>


namespace gh
{

ProviderItem::ProviderItem(const Response &r)
    : QStandardItem(r.name), m_data(r)
{
    if (r.kind == Private)
        setIcon(KIcon("github-private"));
    else if (r.kind == Fork)
        setIcon(KIcon("github-forked"));
    else
        setIcon(KIcon("github-repo"));
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
