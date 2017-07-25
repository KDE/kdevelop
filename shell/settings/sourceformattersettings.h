/* This file is part of KDevelop
*  Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

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
