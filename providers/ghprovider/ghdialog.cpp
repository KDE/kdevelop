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


#include <QLabel>

#include <KLocalizedString>
#include <KMessageBox>
#include <KPasswordDialog>

#include <ghdialog.h>
#include <ghaccount.h>
#include <ghresource.h>


#define VALID_ACCOUNT "You're logged in as <b>%1</b>. You can check the " \
    "authorization for this application and others " \
    "<a href=\"https://github.com/settings/applications\">here</a>."
#define INVALID_ACCOUNT "You haven't authorized KDevelop to use your Github " \
    "account. If you authorize KDevelop, you will be able to fetch your " \
    "public/private repositories and the repositories from your organizations."


namespace gh
{

Dialog::Dialog(QWidget *parent, Account *account) : KDialog(parent)
{
    m_account = account;
    m_name = "";

    if (m_account->validAccount()) {
        QString str = QString(VALID_ACCOUNT).arg(m_account->name());
        m_text = new QLabel(i18n(str.toUtf8()), this);
        setButtons(KDialog::User2 | KDialog::User3);
        setButtonText(KDialog::User2, i18n("Log out"));
        setButtonIcon(KDialog::User2, QIcon::fromTheme("dialog-cancel"));
        connect(this, SIGNAL(user2Clicked()), SLOT(revokeAccess()));
        setButtonIcon(KDialog::User3, QIcon::fromTheme("view-refresh"));
        setButtonText(KDialog::User3, i18n("Force sync"));
        connect(this, SIGNAL(user3Clicked()), SLOT(syncUser()));
    } else {
        m_text = new QLabel(i18n(INVALID_ACCOUNT), this);
        setButtons(KDialog::User1 | KDialog::Cancel);
        setButtonText(KDialog::User1, i18n("Authorize"));
        setButtonIcon(KDialog::User1, QIcon::fromTheme("dialog-ok"));
        connect(this, SIGNAL(user1Clicked()), SLOT(authorizeClicked()));
    }

    m_text->setWordWrap(true);
    m_text->setOpenExternalLinks(true);
    setMinimumWidth(350);
    setMainWidget(m_text);
    setCaption(i18n("Github Account"));
}

void Dialog::authorizeClicked()
{
    KPasswordDialog dlg(this, KPasswordDialog::ShowUsernameLine);
    dlg.setPrompt(i18n("Enter a login and a password"));
    if(!dlg.exec())
        return;

    m_text->setAlignment(Qt::AlignCenter);
    m_text->setText(i18n("Waiting for response"));
    m_account->setName(dlg.username());
    Resource *rs = m_account->resource();
    rs->authenticate(dlg.username(), dlg.password());
    connect(rs, SIGNAL(authenticated(QByteArray, QByteArray)),
            SLOT(authorizeResponse(QByteArray, QByteArray)));
}

void Dialog::authorizeResponse(const QByteArray &id, const QByteArray &token)
{
    if (id.isEmpty()) {
        m_text->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_text->setText(i18n(INVALID_ACCOUNT));
        m_account->setName("");
        KMessageBox::sorry(this, i18n("Authentication failed! Please, "
                                      "try again"));
        return;
    }
    m_account->saveToken(id, token);
    syncUser();
}

void Dialog::syncUser()
{
    Resource *rs = m_account->resource();
    connect(rs, SIGNAL(orgsUpdated(QStringList)),
            SLOT(updateOrgs(QStringList)));
    m_text->setAlignment(Qt::AlignCenter);
    m_text->setText(i18n("Waiting for response"));
    rs->getOrgs(m_account->token());
}

void Dialog::updateOrgs(const QStringList orgs)
{
    if (!orgs.isEmpty())
        m_account->setOrgs(orgs);
    emit shouldUpdate();
    close();
}

void Dialog::revokeAccess()
{
    KPasswordDialog dlg(this);
    dlg.setPrompt(i18n("Please, write your password here."));
    if(!dlg.exec())
        return;
    m_account->invalidate(dlg.password());
    emit shouldUpdate();
    close();
}

} // End of namespace gh
