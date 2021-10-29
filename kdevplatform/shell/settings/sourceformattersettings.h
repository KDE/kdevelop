/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SOURCEFORMATTERSETTINGS_H
#define KDEVPLATFORM_SOURCEFORMATTERSETTINGS_H

#include <interfaces/configpage.h>

#include "ui_sourceformattersettings.h"

/** \short The settings modulefor the Source formatter plugin.
* It supports predefined and custom styles. A live preview of the style
* is shown on the right side of the page.s
*/
class SourceFormatterSettings : public KDevelop::ConfigPage, public Ui::SourceFormatterSettingsUI
{
Q_OBJECT

public:
    explicit SourceFormatterSettings(QWidget* parent = nullptr);
    ~SourceFormatterSettings() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public Q_SLOTS:
    void reset() override;
    void apply() override;
    void defaults() override;

private:
};

#endif // KDEVPLATFORM_SOURCEFORMATTERSETTINGS_H
