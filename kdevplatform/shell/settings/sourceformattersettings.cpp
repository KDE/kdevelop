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
#include "sourceformattersettings.h"

#include <KLocalizedString>
#include <KConfigGroup>

#include <shell/core.h>
#include <shell/sourceformattercontroller.h>

#include "debug.h"

using namespace KDevelop;

SourceFormatterSettings::SourceFormatterSettings(QWidget* parent)
    : KDevelop::ConfigPage(nullptr, nullptr, parent)
{
    setupUi(this);

    connect(formatterSelectionEdit, &SourceFormatterSelectionEdit::changed,
            this, &SourceFormatterSettings::changed);
}

SourceFormatterSettings::~SourceFormatterSettings()
{
}

void SourceFormatterSettings::reset()
{
    KConfigGroup sessionConfig = Core::self()->sourceFormatterControllerInternal()->sessionConfig();

    bool b = blockSignals( true );
    chkKateModelines->blockSignals( !b );
    chkKateOverrideIndentation->blockSignals( !b );
    chkKateModelines->setChecked(sessionConfig.readEntry(SourceFormatterController::kateModeLineConfigKey(), false));
    chkKateOverrideIndentation->setChecked(sessionConfig.readEntry(SourceFormatterController::kateOverrideIndentationConfigKey(), false));
    blockSignals( b );
    chkKateModelines->blockSignals( b );
    chkKateOverrideIndentation->blockSignals( b );

    formatterSelectionEdit->loadSettings(sessionConfig);
}

void SourceFormatterSettings::apply()
{
    KConfigGroup sessionConfig = Core::self()->sourceFormatterControllerInternal()->sessionConfig();

    sessionConfig.writeEntry( SourceFormatterController::kateModeLineConfigKey(), chkKateModelines->isChecked() );
    sessionConfig.writeEntry( SourceFormatterController::kateOverrideIndentationConfigKey(), chkKateOverrideIndentation->isChecked() );

    formatterSelectionEdit->saveSettings(sessionConfig);

    sessionConfig.sync();

    Core::self()->sourceFormatterControllerInternal()->settingsChanged();
}

void SourceFormatterSettings::defaults()
{
    // do nothing
}


QString SourceFormatterSettings::name() const
{
    return i18n("Source Formatter");
}

QString SourceFormatterSettings::fullName() const
{
    return i18n("Configure Source Formatter");
}

QIcon SourceFormatterSettings::icon() const
{
    return QIcon::fromTheme(QStringLiteral("text-field"));
}
