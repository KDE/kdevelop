/*
    SPDX-FileCopyrightText: 2012-2013 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit Account(Resource *resource);

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
