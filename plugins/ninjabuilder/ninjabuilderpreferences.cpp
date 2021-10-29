/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ninjabuilderpreferences.h"

#include "ui_ninjaconfig.h"
#include "ninjabuilderconfig.h"

using namespace KDevelop;

NinjaBuilderPreferences::NinjaBuilderPreferences(IPlugin* plugin,
                                                 const ProjectConfigOptions& options,
                                                 QWidget* parent)
    : ProjectConfigPage<NinjaBuilderSettings>(plugin, options, parent)
{
    m_prefsUi = new Ui::NinjaConfig;
    m_prefsUi->setupUi(this);

    connect(m_prefsUi->configureEnvironment, &EnvironmentConfigureButton::environmentConfigured,
            this, &NinjaBuilderPreferences::changed);

    m_prefsUi->configureEnvironment->setSelectionWidget(m_prefsUi->kcfg_environmentProfile);
}

NinjaBuilderPreferences::~NinjaBuilderPreferences()
{
    delete m_prefsUi;
}

QString NinjaBuilderPreferences::name() const
{
    return i18nc("@title:tab", "Ninja");
}

QString NinjaBuilderPreferences::fullName() const
{
    return i18nc("@title:tab", "Configure Ninja settings");
}

QIcon NinjaBuilderPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("run-build"));
}
