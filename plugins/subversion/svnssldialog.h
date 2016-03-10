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

#include <QDialog>
#include <QDialogButtonBox>

class SvnSSLTrustDialog: public QDialog
{
    Q_OBJECT
public:
    explicit SvnSSLTrustDialog( QWidget *parent = nullptr );
    ~SvnSSLTrustDialog() override;

    void setCertInfos( const QString& hostname, const QString& fingerPrint,
                       const QString& validfrom, const QString& validuntil,
                       const QString& issuerName, const QString& realm,
                       const QStringList& failures );
    bool useTemporarily();
private slots:
    void buttonClicked(QAbstractButton *button);
private:
    class SvnSSLTrustDialogPrivate *d;
    QDialogButtonBox *buttonBox;
};

#endif
