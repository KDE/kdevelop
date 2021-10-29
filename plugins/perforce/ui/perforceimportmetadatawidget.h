/*
    SPDX-FileCopyrightText: 2018 Morten Danielsen Volden

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PERFORCEIMPORTMETADATAWIDGET_H
#define KDEVPLATFORM_PERFORCEIMPORTMETADATAWIDGET_H

#include "ui_perforceimportmetadatawidget.h"

#include <vcs/widgets/vcsimportmetadatawidget.h>

/**
 * Asks the user for all options needed to import an existing directory into
 * a Perforce repository
 */
class PerforceImportMetadataWidget
    : public KDevelop::VcsImportMetadataWidget
{
    Q_OBJECT

public:
    explicit PerforceImportMetadataWidget(QWidget* parent = nullptr);

    QUrl source() const override;
    KDevelop::VcsLocation destination() const override;
    //  TODO: Is not used, it returns an empty string
    QString message() const override;
    void setSourceLocation(const KDevelop::VcsLocation&) override;
    void setSourceLocationEditable(bool) override;
    void setMessage(const QString& message) override;
    bool hasValidData() const override;

private Q_SLOTS:
    void testP4setup();

private:

    bool validateP4executable();

    bool validateP4port(const QString& projectDir) const;

    bool validateP4user(const QString& projectDir) const;

    Ui::PerforceImportMetadataWidget* m_ui;
    QString m_errorDescription;
};

#endif
