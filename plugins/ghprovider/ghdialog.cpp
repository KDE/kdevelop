/*
    SPDX-FileCopyrightText: 2012-2013 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ghdialog.h"

#include <ghaccount.h>
#include <ghresource.h>

#include <KLocalizedString>
#include <KMessageBox>
#include <KPasswordDialog>

#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPointer>
#include <QInputDialog>

static QString invalidAccountText()
{
    return i18n("You have not authorized KDevelop to use your GitHub account. "
                "If you authorize KDevelop, you will be able to fetch your "
                "public/private repositories and the repositories from your "
                "organizations.");
}

static QString tokenLinkStatementText()
{
    return i18nc("%1 is the URL with the GitHub token settings",
                 "You can check the authorization for this application and "
                 "others at %1",
                 QStringLiteral("https://github.com/settings/tokens."));
}

namespace gh
{

Dialog::Dialog(QWidget *parent, Account *account)
    : QDialog(parent)
    , m_account(account)
{
    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    auto buttonBox = new QDialogButtonBox();

    if (m_account->validAccount()) {
        m_text = new QLabel(i18n("You are logged in as <b>%1</b>.<br/>%2",
                                 m_account->name(), tokenLinkStatementText()), this);

        auto logOutButton = new QPushButton;
        logOutButton->setText(i18nc("@action:button", "Log Out"));
        logOutButton->setIcon(QIcon::fromTheme(QStringLiteral("dialog-cancel")));
        buttonBox->addButton(logOutButton, QDialogButtonBox::ActionRole);
        connect(logOutButton, &QPushButton::clicked, this, &Dialog::revokeAccess);

        auto forceSyncButton = new QPushButton;
        forceSyncButton->setText(i18nc("@action:button", "Force Sync"));
        forceSyncButton->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
        buttonBox->addButton(forceSyncButton, QDialogButtonBox::ActionRole);
        connect(forceSyncButton, &QPushButton::clicked, this, &Dialog::syncUser);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    } else {
        m_text = new QLabel(invalidAccountText(), this);

        buttonBox->addButton(QDialogButtonBox::Cancel);

        auto authorizeButton = new QPushButton;
        buttonBox->addButton(authorizeButton, QDialogButtonBox::ActionRole);
        authorizeButton->setText(i18nc("@action:button", "Authorize"));
        authorizeButton->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok")));
        connect(authorizeButton, &QPushButton::clicked, this, &Dialog::authorizeClicked);
    }

    m_text->setWordWrap(true);
    m_text->setOpenExternalLinks(true);
    setMinimumWidth(350);
    mainLayout->addWidget(m_text);

    mainLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setWindowTitle(i18nc("@title:window", "GitHub Account"));
}

void Dialog::authorizeClicked()
{
    QPointer<KPasswordDialog> dlg = new KPasswordDialog(this, KPasswordDialog::ShowUsernameLine);
    dlg->setPrompt(i18n("Enter a login and a password"));
    if(dlg->exec()) {
        m_text->setAlignment(Qt::AlignCenter);
        m_text->setText(i18n("Waiting for response"));
        m_account->setName(dlg->username());

        Resource *rs = m_account->resource();
        rs->authenticate(dlg->username(), dlg->password());
        connect(rs, &Resource::twoFactorAuthRequested,
                this, &Dialog::twoFactorResponse);
        connect(rs, &Resource::authenticated,
                this, &Dialog::authorizeResponse);
    }
    delete dlg;
}

void Dialog::authorizeResponse(const QByteArray &id, const QByteArray &token, const QString &tokenName)
{
    Q_UNUSED(tokenName);

    Resource *rs = m_account->resource();
    disconnect(rs, &Resource::authenticated,
               this, &Dialog::authorizeResponse);

    if (id.isEmpty()) {
        m_text->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_text->setText(invalidAccountText());
        m_account->setName(QString());
        KMessageBox::error(this, i18n("Authentication failed. Please try again.\n\n"
                                      "Could not create token: \"%1\"\n%2", tokenName,
                                      tokenLinkStatementText()),
                                 i18nc("@title:window", "GitHub Authorization Failed"));
        return;
    }
    else{
        KMessageBox::information(this, i18n("Authentication succeeded.\n\n"
                                            "Created token: \"%1\"\n%2", tokenName,
                                            tokenLinkStatementText()),
                                       i18nc("@title:window", "GitHub Account Authorized"));
    }
    m_account->saveToken(id, token);
    syncUser();
}

void Dialog::twoFactorResponse(const QString &transferHeader)
{
    auto code = QInputDialog::getText(this, i18nc("@title:window", "Authentication Code"), i18nc("@label:textbox", "OTP Code:"));
    Resource* rs = m_account->resource();
    disconnect(rs, &Resource::twoFactorAuthRequested,
               this, &Dialog::twoFactorResponse);
    rs->twoFactorAuthenticate(transferHeader, code);
}

void Dialog::syncUser()
{
    Resource *rs = m_account->resource();
    connect(rs, &Resource::orgsUpdated,
            this, &Dialog::updateOrgs);
    m_text->setAlignment(Qt::AlignCenter);
    m_text->setText(i18n("Waiting for response"));
    rs->getOrgs(m_account->token());
}

void Dialog::updateOrgs(const QStringList& orgs)
{
    Resource *rs = m_account->resource();
    disconnect(rs, &Resource::orgsUpdated,
              this, &Dialog::updateOrgs);

    if (!orgs.isEmpty())
        m_account->setOrgs(orgs);
    emit shouldUpdate();
    close();
}

void Dialog::revokeAccess()
{
    QPointer<KPasswordDialog> dlg = new KPasswordDialog(this);
    dlg->setPrompt(i18n("Please, write your password here."));
    if(dlg->exec()) {
        m_account->invalidate(dlg->password());
        emit shouldUpdate();
        close();
    }
    delete dlg;
}

} // End of namespace gh
