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


#ifndef GH_PROVIDERMODEL_H
#define GH_PROVIDERMODEL_H


#include <QtCore/QUrl>
#include <QStandardItemModel>

namespace gh
{

/// Convenient enum to define the kind of the repo.
enum Repo {
    Public = 0,
    Private,
    Fork
};

/// Basic struct that represents a response from Github.
struct Response {
    /// The name of the repo.
    QString name;

    /// The url of the repo.
    QUrl url;

    /// The kind of the repo (public, private, fork).
    enum Repo kind;
};

/**
 * @class ProviderItem
 * This class represents an item that is contained in the main list view
 * and that stores a response from Github.
 */
class ProviderItem : public QStandardItem
{
public:
    /// Constructor. \p r The response that this item stores.
    ProviderItem(const Response &r);

    /// Re-implemented from QStandardItem.
    virtual QVariant data(int role = Qt::UserRole + 1) const;

private:
    Response m_data;
};

/**
 * @class ProviderModel
 * The model to be used in the main list view.
 */
class ProviderModel : public QStandardItemModel
{
    Q_OBJECT

public:
    enum Role { VcsLocationRole = Qt::UserRole + 1 };

    /// Constructor.
    explicit ProviderModel(QObject *parent = NULL);
};

} // End of namespace gh


#endif // GH_PROVIDERMODEL_H
