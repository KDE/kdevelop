/*
    SPDX-FileCopyrightText: 2012-2013 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GH_DIALOG_H
#define GH_DIALOG_H


#include <QDialog>

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
class Dialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param parent The QWidget this Dialog is parented to.
     * @param account The user's account.
     */
    explicit Dialog(QWidget *parent, Account *account);

Q_SIGNALS:
    /**
     * This signal is emitted whenever the dialog has successfully performed
     * an action that may imply some changes on the UI of the plugin.
     */
    void shouldUpdate();

private Q_SLOTS:
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
    void authorizeResponse(const QByteArray &id, const QByteArray &token, const QString &tokenName);

    /**
     * Handle a two factor response from GitHub during the authorization process.
     */
    void twoFactorResponse(const QString &transferHeader);

    /// Sync the user's groups.
    void syncUser();

    /**
     * Update the organization list of the current user.
     *
     * @param orgs A list of organizations for the current user.
     */
    void updateOrgs(const QStringList& orgs);

    /// Revoke the access of this application.
    void revokeAccess();

private:
    Account *m_account;
    QString m_name;
    QLabel *m_text;
};

} // End of namespace gh


#endif /* GH_DIALOG_H */
