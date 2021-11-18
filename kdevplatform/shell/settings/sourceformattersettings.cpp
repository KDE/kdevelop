/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
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

    // Widgets are managed manually, so we must notify ConfigDialog when something
    // changes. Otherwise it will not enable "Apply" button and won't call apply().
    for (auto* checkbox : {chkKateOverrideIndentation, chkKateModelines}) {
        connect(checkbox, &QCheckBox::toggled, this, &SourceFormatterSettings::changed);
    }
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
