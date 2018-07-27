/***************************************************************************
*   This file is part of KDevelop Perforce plugin, KDE project            *
*                                                                         *
*   Copyright 2018  Morten Danielsen Volden                               *
*                                                                         *
*   This program is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation, either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
***************************************************************************/

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
