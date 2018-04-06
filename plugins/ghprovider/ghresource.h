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


#ifndef GH_RESOURCE_H
#define GH_RESOURCE_H


#include <QObject>

class KJob;
namespace KIO {
    class Job;
    class TransferJob;
}

namespace gh
{
class ProviderModel;

/**
 * @class Resource
 *
 * This class provides methods that extract information from the given
 * Github's JSON responses.
 */
class Resource : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param parent The QObject this Resource is parented to.
     * @param model The model to be used in the other methods.
     */
    explicit Resource(QObject *parent, ProviderModel *model);

    /**
     * Search repos by calling the Github API. When successful, it will re-fill
     * the model provided in the constructor. Otherwise, if something went
     * wrong, it will do nothing.
     *
     * @param uri A string containing the URI to be called to retrieve
     * the repos. Therefore, this parameter also determines whether the repos
     * belong to a user or a organization.
     * @param token The authorization token.
     */
    void searchRepos(const QString &uri, const QString &token);

    /**
     * Get public and private organizations for the currently authenticated
     * user and save it to its local account.
     *
     * @param token The authorization token.
     */
    void getOrgs(const QString &token);

    /**
     * Authenticate the current user with the given name and password.
     * This method will eventually emit the authenticated signal.
     *
     * @param name The username of the current user.
     * @param password The password used to login the current user.
     */
    void authenticate(const QString &name, const QString &password);

    /**
     * Authenticate the current user with the two factor authentication code.
     * Must be called after a call to authenticate.
     *
     * @param code The two factor authentication code.
     */
    void twoFactorAuthenticate(const QString &transferHeader, const QString &code);

    /**
     * Revoke an access to the Github API (a.k.a. log out the current user).
     *
     * @param id The id of the currently valid authorization.
     * @param name The name of the current user.
     * @param password The Github API requires the password again to perform
     * this action.
     */
    void revokeAccess(const QString &id, const QString &name, const QString &password);

private:
    /**
     * Get a TransferJob for the given path and authorization token.
     *
     * @param path The path to be requested (i.e. "/user/orgs")
     * @param token The authorization token to be set on the headers.
     */
    KIO::TransferJob * getTransferJob(const QString &path, const QString &token) const;

    /**
     * Retrieve the info of the name and the URL of each repo and append
     * it in the model that was provided in the constructor.
     *
     * @param data A Github response in JSON format.
     */
    void retrieveRepos(const QByteArray &data);

    /**
     * Retrieve the organizations for the current user from the given
     * JSON data. It will emit the orgsUpdated signal.
     *
     * @param data A Github response in JSON format.
     */
    void retrieveOrgs(const QByteArray &data);

Q_SIGNALS:
    /**
     * This signal will eventually be sent when the authentication process
     * has finished. An empty id means that the authentication wasn't
     * successful.
     *
     * @param id The id of the authorization. Empty if something went wrong.
     * @param token The authorization token (e.g. "14bf8e87e2ec5fe30f8a6755bda63b5bc4d02e22").
     *   Empty if something went wrong.
     * @param tokenName The authorization token (e.g. "KDevelop Github Provider : machinename - Thu Nov 17 23:18:03 2016 GMT").
     *   Empty if something went wrong.
     */
    void authenticated(const QByteArray &id, const QByteArray &token, const QString &tokenName);

    /**
     * This signal is sent if two factor authentication is requested by GitHub.
     */
    void twoFactorAuthRequested(const QString &transferHeader);

    /**
     * This signal is emitted when the model containing repos has
     * been updated.
     */
    void reposUpdated();

    /**
     * This signal is fired when the list of organizations for the current
     * user has been updated.
     *
     * @param orgs A list of the names of the organizations.
     */
    void orgsUpdated(const QStringList &orgs);

private Q_SLOTS:
    /**
     * Handle the response of the Github authentication process.
     *
     * @param job The KJob responsible from the authentication.
     */
    void slotAuthenticate(KJob *job);

    /**
     * Handle the response of retrieving repos from Github.
     *
     * @param job The job returned after the HTTP request has finished.
     * @param data The data returned by the HTTP response.
     */
    void slotRepos(KIO::Job *job, const QByteArray &data);

    /**
     * Handle the response of retrieving orgs for the currently
     * authenticated user.
     *
     * @param job The job returned after the HTTP request has finished.
     * @param data The data returned by the HTTP response.
     */
    void slotOrgs(KIO::Job *job, const QByteArray &data);

private:
    ProviderModel *m_model;
    QByteArray m_temp, m_orgTemp;
};

} // End of namespace gh


#endif // GH_RESOURCE_H
