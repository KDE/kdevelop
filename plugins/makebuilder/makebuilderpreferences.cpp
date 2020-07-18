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

#include "makebuilderpreferences.h"

#include "ui_makeconfig.h"
#include "makebuilderconfig.h"

using namespace KDevelop;

MakeBuilderPreferences::MakeBuilderPreferences(IPlugin* plugin, const ProjectConfigOptions& options, QWidget* parent)
    : ProjectConfigPage<MakeBuilderSettings>(plugin, options, parent)
{
    m_prefsUi = new Ui::MakeConfig;
    m_prefsUi->setupUi(this);
    connect(m_prefsUi->makeExecutable, &KUrlRequester::textChanged,
            this, &MakeBuilderPreferences::changed);
    connect(m_prefsUi->makeExecutable, &KUrlRequester::urlSelected,
            this, &MakeBuilderPreferences::changed);
    connect(m_prefsUi->configureEnvironment, &EnvironmentConfigureButton::environmentConfigured,
            this, &MakeBuilderPreferences::changed);

    m_prefsUi->configureEnvironment->setSelectionWidget( m_prefsUi->kcfg_environmentProfile );
}

void MakeBuilderPreferences::reset()
{
    ProjectConfigPage::reset();
    QSignalBlocker sigBlock(this); // don't emit changed signal from m_prefsUi->makeExecutable
    m_prefsUi->makeExecutable->setText(MakeBuilderSettings::self()->makeExecutable());
}

void MakeBuilderPreferences::apply()
{
    MakeBuilderSettings::self()->setMakeExecutable(m_prefsUi->makeExecutable->text());
    MakeBuilderSettings::self()->save(); // TODO: is this needed? KConfigDialogManager should end up calling it
    ProjectConfigPage::apply();
}

void MakeBuilderPreferences::defaults()
{
    MakeBuilderSettings::self()->setDefaults();
    m_prefsUi->makeExecutable->setText(MakeBuilderSettings::self()->makeExecutable());
    ProjectConfigPage::defaults();
}

MakeBuilderPreferences::~MakeBuilderPreferences()
{
    delete m_prefsUi;
}

QString MakeBuilderPreferences::standardMakeExecutable()
{
#ifdef Q_OS_WIN
    if (!QStandardPaths::findExecutable(QStringLiteral("make")).isEmpty())
        return QStringLiteral("make");
    if (!QStandardPaths::findExecutable(QStringLiteral("mingw32-make")).isEmpty())
        return QStringLiteral("mingw32-make");
    return QStringLiteral("nmake");
#else
    return QStringLiteral("make");
#endif
}

QString MakeBuilderPreferences::name() const
{
    return i18nc("@title:tab", "Make");
}

QString MakeBuilderPreferences::fullName() const
{
    return i18nc("@title:tab", "Configure Make Settings");
}

QIcon MakeBuilderPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("run-build"));
}
