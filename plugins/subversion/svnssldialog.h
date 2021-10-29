/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVN_SSLDIALOG_H
#define KDEVPLATFORM_PLUGIN_SVN_SSLDIALOG_H

#include <QDialog>

class QDialogButtonBox;
class QAbstractButton;

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
private Q_SLOTS:
    void buttonClicked(QAbstractButton *button);
private:
    class SvnSSLTrustDialogPrivate *d;
    QDialogButtonBox *buttonBox;
};

#endif
