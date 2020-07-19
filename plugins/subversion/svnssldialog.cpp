/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnssldialog.h"

#include <QDialogButtonBox>
#include <QPushButton>

#include <KLocalizedString>

#include "ui_ssltrustdialog.h"

class SvnSSLTrustDialogPrivate
{
public:
    Ui::SvnSSLTrustDialog ui;
    bool temporarily;
};

SvnSSLTrustDialog::SvnSSLTrustDialog( QWidget *parent )
    : QDialog( parent ), d( new SvnSSLTrustDialogPrivate )
{
    d->ui.setupUi( this );
    d->temporarily = true;
    setWindowTitle(i18nc("@title:window", "SSL Server Certificate"));
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    buttonBox->addButton(i18nc("@action:button", "Trust Permanently"), QDialogButtonBox::YesRole);
    buttonBox->addButton(i18nc("@action:button", "Trust Temporarily"), QDialogButtonBox::AcceptRole)->setDefault(true);
    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::clicked, this, &SvnSSLTrustDialog::buttonClicked);
}
SvnSSLTrustDialog::~SvnSSLTrustDialog()
{
    delete d;
}

void SvnSSLTrustDialog::setCertInfos( const QString& hostname,
                                      const QString& fingerPrint,
                                      const QString& validfrom,
                                      const QString& validuntil,
                                      const QString& issuerName,
                                      const QString& realm,
                                      const QStringList& failures )
{
    if (!failures.isEmpty()) {
        const QString txt = QLatin1String("<ul><li>") +
                            failures.join(QLatin1String("</li><li>")) +
                            QLatin1String("</li></ul>");
        d->ui.reasons->setHtml( txt );
    }

    d->ui.hostname->setText( hostname );
    d->ui.fingerprint->setText( fingerPrint );
    d->ui.validUntil->setText( validuntil );
    d->ui.validFrom->setText( validfrom );
    d->ui.issuer->setText( issuerName );
    setWindowTitle(i18nc("@title:window", "SSL Server Certificate: %1", realm));

}

bool SvnSSLTrustDialog::useTemporarily()
{
    return d->temporarily;
}

void SvnSSLTrustDialog::buttonClicked(QAbstractButton *button)
{
    if (buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
    {
        d->temporarily = true;
    } else
    {
        d->temporarily = false;
    }
    accept();
}
