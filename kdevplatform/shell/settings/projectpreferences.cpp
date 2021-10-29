/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "projectpreferences.h"

#include "projectconfig.h"
#include "../core.h"
#include "../projectcontroller.h"

#include "ui_projectpreferences.h"

namespace KDevelop
{

ProjectPreferences::ProjectPreferences(QWidget* parent)
    : ConfigPage(nullptr, ProjectSettings::self(), parent)
{
    preferencesDialog = new Ui::ProjectPreferences;
    preferencesDialog->setupUi(this);
}

ProjectPreferences::~ProjectPreferences( )
{
    delete preferencesDialog;
}

void ProjectPreferences::apply()
{
    ConfigPage::apply();

    Core::self()->projectControllerInternal()->loadSettings(false);
}

void ProjectPreferences::slotSettingsChanged()
{
    emit changed();
}

QString ProjectPreferences::name() const
{
    return i18n("Projects");
}

QString ProjectPreferences::fullName() const
{
    return i18n("Configure Projects");
}

QIcon ProjectPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("project-open"));
}

}

