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


#ifndef GH_DIALOG_H
#define GH_DIALOG_H


#include <KDialog>

class QLabel;

namespace gh
{
class Account;

/**
 * @class Dialog
 *
 * The Dialog class handles the configuration dialog to be shown so the
 * user can setup its Github account.
 */
class Dialog : public KDialog
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param parent The QWidget this Dialog is parented to.
     * @param account The user's account.
     */
    Dialog(QWidget *parent, Account *account);

signals:
    /**
     * This signal is emitted whenever the dialog has successfully performed
     * an action that may imply some changes on the UI of the plugin.
     */
    void shouldUpdate();

private slots:
    /**
     * The "Authorize" button has been clicked. a KPasswordDialog will be
     * shown and a request will be sent to Github to authenticate the given
     * user.
     */
    void authorizeClicked();

    /**
     * Handle a response from Github on the authorization process. If the
     * given id is empty, it means that something went wrong. Otherwise, when
     * successful, it will also call the syncUser method.
     *
     * @param id The id of the authorization.
     * @param token The authorization token.
     */
    void authorizeResponse(const QByteArray &id, const QByteArray &token);

    /// Sync the user's groups.
    void syncUser();

    /**
     * Update the organization list of the current user.
     *
     * @param orgs A list of organizations for the current user.
     */
    void updateOrgs(const QStringList orgs);

    /// Revoke the access of this application.
    void revokeAccess();

private:
    Account *m_account;
    QString m_name;
    QLabel *m_text;
};

} // End of namespace gh


#endif /* GH_DIALOG_H */
