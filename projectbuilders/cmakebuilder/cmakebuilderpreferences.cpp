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

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <KAboutData>
#include "ui_cmakebuildersettings.h"
#include "cmakebuilderconfig.h"

K_PLUGIN_FACTORY(CMakeBuilderPreferencesFactory, registerPlugin<CMakeBuilderPreferences>(); )
// K_EXPORT_PLUGIN(CMakeBuilderPreferencesFactory("kcm_kdev_cmakebuilder"))

CMakeBuilderPreferences::CMakeBuilderPreferences(QWidget* parent, const QVariantList& args)
    : KCModule( KAboutData::pluginData("kcm_kdev_ninjabuilder"), parent, args)
{
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget;
    m_prefsUi = new Ui::CMakeBuilderConfig;
    m_prefsUi->setupUi( w );
    l->addWidget( w );
#ifdef Q_OS_WIN
    // Visual Studio solution is the standard generator under windows, but we dont want to use
    // the VS IDE, so we need nmake makefiles
    m_prefsUi->generator->addItem("NMake Makefiles");
#else
    m_prefsUi->generator->addItem("Unix Makefiles");
#endif
    bool hasNinja = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IProjectBuilder", "KDevNinjaBuilder");
    if(hasNinja)
        m_prefsUi->generator->addItem("Ninja");
    
    connect(m_prefsUi->generator, SIGNAL(currentIndexChanged(QString)), SLOT(generatorChanged(QString)));
}

CMakeBuilderPreferences::~CMakeBuilderPreferences()
{
    delete m_prefsUi;
}

void CMakeBuilderPreferences::defaults()
{
    m_prefsUi->generator->setCurrentIndex(0);
    KCModule::defaults();
}

void CMakeBuilderPreferences::save()
{
    CMakeBuilderSettings::setGenerator(m_prefsUi->generator->currentText());
    KCModule::save();
    CMakeBuilderSettings::self()->writeConfig();
}

void CMakeBuilderPreferences::load()
{
    int idx = m_prefsUi->generator->findText(CMakeBuilderSettings::self()->generator());
    m_prefsUi->generator->setCurrentIndex(idx);
    KCModule::load();
}

void CMakeBuilderPreferences::generatorChanged(const QString& generator)
{
    emit changed(CMakeBuilderSettings::self()->generator()!=generator);
}

#include "cmakebuilderpreferences.moc"
