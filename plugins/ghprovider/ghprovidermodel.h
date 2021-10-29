/*
    SPDX-FileCopyrightText: 2012-2013 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GH_PROVIDERMODEL_H
#define GH_PROVIDERMODEL_H


#include <QUrl>
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
    explicit ProviderItem(const Response &r);

    /// Re-implemented from QStandardItem.
    QVariant data(int role = Qt::UserRole + 1) const override;

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
    explicit ProviderModel(QObject *parent = nullptr);
};

} // End of namespace gh


#endif // GH_PROVIDERMODEL_H
