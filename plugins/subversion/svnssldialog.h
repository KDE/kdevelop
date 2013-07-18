/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_SVN_SSLDIALOG_H
#define KDEVPLATFORM_PLUGIN_SVN_SSLDIALOG_H

#include <kdialog.h>


class SvnSSLTrustDialog: public KDialog
{
    Q_OBJECT
public:
    SvnSSLTrustDialog( QWidget *parent = 0 );
    virtual ~SvnSSLTrustDialog();

    void setCertInfos( const QString& hostname, const QString& fingerPrint,
                       const QString& validfrom, const QString& validuntil,
                       const QString& issuerName, const QString& realm,
                       const QStringList& failures );
    bool useTemporarily();
private slots:
    void temporarilyClicked();
    void permanentlyClicked();
private:
    class SvnSSLTrustDialogPrivate *d;
};

#endif
