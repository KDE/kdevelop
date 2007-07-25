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

#ifndef SVN_AUTHDIALOG_H
#define SVN_AUTHDIALOG_H

#include "ui_svnssltrustdialog.h"
#include "ui_svnlogindialog.h"

struct svn_auth_ssl_server_cert_info_t;

class SvnSSLTrustDialog: public KDialog
{
    Q_OBJECT
public:
    SvnSSLTrustDialog( QWidget *parent = 0 );
    virtual ~SvnSSLTrustDialog();

    void setFailedReasons( unsigned int failures );
    void setCertInfos( const svn_auth_ssl_server_cert_info_t *ci );
    void setCertInfos( const QString& hostname, const QString& fingerPrint,
                       const QString& validfrom, const QString& validuntil,
                       const QString& issuerName, const QString& ascii_cert);
    int userDecision();

protected Q_SLOTS:
    virtual void accept();
    virtual void reject();

private:
    class SvnSSLTrustDialogPrivate *d;
};

class SvnLoginDialog : public KDialog
{
    Q_OBJECT
public:
    SvnLoginDialog( QWidget *parent = 0 );
    virtual ~SvnLoginDialog();

    void setRealm( QString &realm );
    QString userName();
    QString passWord();
    bool save();

private:
    class SvnLoginDialogPrivate *d;

};

#endif
