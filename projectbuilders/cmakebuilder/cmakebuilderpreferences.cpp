/* KDevelop CCMake Support
 *
 * Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#include "cmakebuilderpreferences.h"

#include <QVBoxLayout>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>

#include "ui_cmakebuilderpreferences.h"
#include "cmakebuilderconfig.h"
#include "cmakeutils.h"

CMakeBuilderPreferences::CMakeBuilderPreferences(KDevelop::IPlugin* plugin, QWidget* parent)
    : KDevelop::ConfigPage(plugin, CMakeBuilderSettings::self(), parent)
{
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    m_prefsUi = new Ui::CMakeBuilderPreferences;
    m_prefsUi->setupUi( w );
    l->addWidget( w );

#ifdef Q_OS_WIN
    // Visual Studio solution is the standard generator under windows, but we dont want to use
    // the VS IDE, so we need nmake makefiles
    m_prefsUi->kcfg_generator->addItem("NMake Makefiles");
    static_cast<KConfigSkeleton::ItemString *>(CMakeBuilderSettings::self()->findItem("generator"))->setDefaultValue("NMake Makefiles");

    m_prefsUi->kcfg_cmakeExe->setFilter("*.exe");
#else
    m_prefsUi->kcfg_generator->addItem("Unix Makefiles");
#endif

    bool hasNinja = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IProjectBuilder", "KDevNinjaBuilder");
    if(hasNinja)
        m_prefsUi->kcfg_generator->addItem("Ninja");

    connect(m_prefsUi->kcfg_generator, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &CMakeBuilderPreferences::generatorChanged);
}

CMakeBuilderPreferences::~CMakeBuilderPreferences()
{
    delete m_prefsUi;
}

void CMakeBuilderPreferences::defaults()
{
    m_prefsUi->kcfg_generator->setCurrentIndex(0);
    KDevelop::ConfigPage::defaults();
}

void CMakeBuilderPreferences::apply()
{
    CMakeBuilderSettings::setGenerator(m_prefsUi->kcfg_generator->currentText());
    KDevelop::ConfigPage::apply();
    CMakeBuilderSettings::self()->save();
}

void CMakeBuilderPreferences::reset()
{
    int idx = m_prefsUi->kcfg_generator->findText(CMakeBuilderSettings::self()->generator());
    m_prefsUi->kcfg_generator->setCurrentIndex(idx);
    KDevelop::ConfigPage::reset();
}

void CMakeBuilderPreferences::generatorChanged(const QString& generator)
{
    if (CMakeBuilderSettings::self()->generator() != generator) {
        emit changed();
    }
}

QString CMakeBuilderPreferences::name() const
{
    return i18n("CMake");
}

QString CMakeBuilderPreferences::fullName() const
{
    return i18n("Configure global CMake settings");
}

QIcon CMakeBuilderPreferences::icon() const
{
    return QIcon::fromTheme("cmake");
}
