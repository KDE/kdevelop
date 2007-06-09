/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svn_authdialog.h"
#include <QTreeWidgetItem>
extern "C" {
#include <svn_auth.h>
#include <svn_client.h>
}
#include <klocale.h>

class SvnSSLTrustDialogPrivate
{
public:
    int m_decision;
    Ui::SvnSSLTrustDialog ui;
};

SvnSSLTrustDialog::SvnSSLTrustDialog( QWidget *parent )
    : KDialog( parent ), d( new SvnSSLTrustDialogPrivate )
{
    QWidget *widget = new QWidget( this );
    setMainWidget( widget );
    setButtons( KDialog::Ok | KDialog::Cancel );
    d->ui.setupUi( widget );
    d->ui.msgLabel->setText( i18n("Fail to automatically validiate certificate from server.") );
//     connect( okBtn, SIGNAL(clicked()), this, SLOT(slotAccept()) );
//     connect( cancelBtn, SIGNAL(clicked()), this, SLOT(slotReject()) );
}
SvnSSLTrustDialog::~SvnSSLTrustDialog()
{
    delete d;
}

/** Certificate is not yet valid. */
#define SVN_AUTH_SSL_NOTYETVALID 0x00000001
/** Certificate has expired. */
#define SVN_AUTH_SSL_EXPIRED     0x00000002
/** Certificate's CN (hostname) does not match the remote hostname. */
#define SVN_AUTH_SSL_CNMISMATCH  0x00000004
/** Certificate authority is unknown (i.e. not trusted) */
#define SVN_AUTH_SSL_UNKNOWNCA   0x00000008
/** Other failure. This can happen if neon has introduced a new
 * failure bit that we do not handle yet. */
#define SVN_AUTH_SSL_OTHER       0x40000000

void SvnSSLTrustDialog::setFailedReasons( unsigned int failures )
{
    QString errs;
    if( failures & SVN_AUTH_SSL_NOTYETVALID )
        errs += i18n("Certificate is not yet valid");
    if( failures & SVN_AUTH_SSL_EXPIRED )
        errs += '\n' + i18n("Certificate has expired.");
    if( failures & SVN_AUTH_SSL_CNMISMATCH )
        errs += '\n' + i18n("Certificate's CN (hostname) does not match the remote hostname.");
    if( failures & SVN_AUTH_SSL_UNKNOWNCA )
        errs += '\n' + i18n("Certificate authority is unknown");
    if( failures & SVN_AUTH_SSL_OTHER )
        errs += '\n' + i18n("Other unknown failures");
    d->ui.reasonLabel->setText( errs );
}
void SvnSSLTrustDialog::setCertInfos( const svn_auth_ssl_server_cert_info_t *ci )
{
    setCertInfos( ci->hostname, ci->fingerprint,ci->valid_from, ci->valid_until, ci->issuer_dname, ci->ascii_cert );
}

void SvnSSLTrustDialog::setCertInfos( const QString& hostname,
                                      const QString& fingerPrint,
                                      const QString& validfrom,
                                      const QString& validuntil,
                                      const QString& issuerName,
                                      const QString& ascii_cert)
{
    QList<QTreeWidgetItem*> items;
    QStringList list1;
    list1 << "Hostname" << hostname;
    QStringList list2;
    list2 << "Fingerprint" << fingerPrint;
    QStringList list3;
    list3 << "Valid From" << validfrom;
    QStringList list4;
    list4 << "Valid Until" << validuntil;
    QStringList list5;
    list5 << "Issuer Name" << issuerName;
    QStringList list6;
    list6 << "Ascii Cert" << ascii_cert;

    QTreeWidgetItem *item1 = new QTreeWidgetItem((QTreeWidget*)0, list1);
    items.append(item1);
    QTreeWidgetItem *item2 = new QTreeWidgetItem((QTreeWidget*)0, list2);
    items.append(item2);
    QTreeWidgetItem *item3 = new QTreeWidgetItem((QTreeWidget*)0, list3);
    items.append(item3);
    QTreeWidgetItem *item4 = new QTreeWidgetItem((QTreeWidget*)0, list4);
    items.append(item4);
    QTreeWidgetItem *item5 = new QTreeWidgetItem((QTreeWidget*)0, list5);
    items.append(item5);
    QTreeWidgetItem *item6 = new QTreeWidgetItem((QTreeWidget*)0, list6);
    items.append(item6);

    d->ui.treeWidget->insertTopLevelItems(0, items);
}

int SvnSSLTrustDialog::userDecision()
{
    return d->m_decision;
}

void SvnSSLTrustDialog::accept()
{
    if( d->ui.permanentCheck->isChecked() ){
        d->m_decision = 1;
    } else{
        d->m_decision = 0;
    }
    KDialog::accept();
}
void SvnSSLTrustDialog::reject()
{
    d->m_decision = -1;
    KDialog::reject();
}
//////////////////////////////////////////////////////////////

class SvnLoginDialogPrivate
{
public:
    Ui::SvnLoginDialog ui;
};

SvnLoginDialog::SvnLoginDialog( QWidget *parent )
    : KDialog( parent ), d( new SvnLoginDialogPrivate )
{
    QWidget *widget = new QWidget( this );
    setMainWidget( widget );
    setButtons( KDialog::Ok | KDialog::Cancel );
    d->ui.setupUi( widget );
}
SvnLoginDialog::~SvnLoginDialog()
{
    delete d;
}

void SvnLoginDialog::setRealm( QString &realm )
{
    QString msg;
    msg = i18n("ID and Password for the following repository");
    msg = msg + "\n\n" + realm + '\n';
    d->ui.realmLabel->setText( msg );
}

QString SvnLoginDialog::userName()
{
    return d->ui.userEdit->text();
}

QString SvnLoginDialog::passWord()
{
    return d->ui.pwdEdit->text();
}

bool SvnLoginDialog::save()
{
    return d->ui.checkBox->isChecked();
}

#include "svn_authdialog.moc"

