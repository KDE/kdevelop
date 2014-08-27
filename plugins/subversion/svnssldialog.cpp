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
#include <QTreeWidgetItem>
#include <klocale.h>

#include "ui_ssltrustdialog.h"

class SvnSSLTrustDialogPrivate
{
public:
    Ui::SvnSSLTrustDialog ui;
    bool temporarily;
};

SvnSSLTrustDialog::SvnSSLTrustDialog( QWidget *parent )
    : KDialog( parent ), d( new SvnSSLTrustDialogPrivate )
{
    d->ui.setupUi( mainWidget() );
    d->temporarily = true;
    setCaption( i18n( "Ssl Server Certificate" ) );
    setButtons( KDialog::User1 | KDialog::User2 | KDialog::Cancel );
    setDefaultButton( KDialog::User2 );
    setButtonText( KDialog::User2, i18n("Trust Temporarily") );
    setButtonText( KDialog::User1, i18n("Trust Permanently") );
    connect( this, SIGNAL(user1Clicked()), this, SLOT(permanentlyClicked()) );
    connect( this, SIGNAL(user2Clicked()), this, SLOT(temporarilyClicked()) );
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
    QString txt = "<ul>";
    foreach( const QString &fail, failures )
    {
        txt += "<li>"+fail+"</li>";
    }
    d->ui.reasons->setHtml( txt );

    d->ui.hostname->setText( hostname );
    d->ui.fingerprint->setText( fingerPrint );
    d->ui.validUntil->setText( validuntil );
    d->ui.validFrom->setText( validfrom );
    d->ui.issuer->setText( issuerName );
    setCaption( i18n( "Ssl Server Certificate: %1", realm ) );

}

bool SvnSSLTrustDialog::useTemporarily()
{
    return d->temporarily;
}

void SvnSSLTrustDialog::temporarilyClicked()
{
    d->temporarily = true;
    accept();
}

void SvnSSLTrustDialog::permanentlyClicked()
{
    d->temporarily = false;
    accept();
}

#include "svnssldialog.moc"

