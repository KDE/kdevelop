/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
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
