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


#ifndef GH_ACCOUNT_H
#define GH_ACCOUNT_H


#include <KConfigGroup>


namespace gh
{
class Resource;

/**
 * @class Account
 *
 * This class holds the configuration of the Github account.
 */
class Account
{
public:
    /**
     * Constructor.
     *
     * @param resource The Github resource.
     */
    Account(Resource *resource);

    /**
     * Invalidate the current account.
     *
     * @param password The password for the current user. The Github API
     * requires it when revoking an access token.
     */
    void invalidate(const QString &password);

    /// @returns true if the current account is still valid.
    bool validAccount() const;

    /// @returns the Github resource.
    inline Resource *resource() const
    {
        return m_resource;
    }

    /// Set the user's name to @p name.
    void setName(const QString &name);

    /// @returns the user's name.
    const QString name() const;

    /// Set the public and private orgs for the current user.
    void setOrgs(const QStringList &orgs);

    /// @returns the public and private orgs for the current user.
    const QStringList orgs() const;

    /**
     * Saves the authorization token.
     *
     * @param id The id of the authorization.
     * @param token The authorization token itself.
     */
    void saveToken(const QByteArray &id, const QByteArray &token);

    /// @returns the authorization token.
    const QString token() const;

private:
    Resource *m_resource;
    KConfigGroup m_group;
};

} // End of namespace gh


#endif /* GH_ACCOUNT_H */
