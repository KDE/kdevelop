/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#include "uipreferences.h"

#include <QVBoxLayout>

#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <kpluginloader.h>

#include "../core.h"
#include "../mainwindow.h"
#include "../uicontroller.h"
#include "ui_uiconfig.h"
#include "uiconfig.h"

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(UiPreferencesFactory, "kcm_kdev_uisettings.json", registerPlugin<UiPreferences>();)
// K_EXPORT_PLUGIN(UiPreferencesFactory(KAboutData("kcm_kdev_uisettings", "kdevplatform", ki18n("User Interface Settings"), "0.1")))

UiPreferences::UiPreferences(QWidget* parent, const QVariantList& args )
    : KCModule( KAboutData::pluginData("kcm_kdev_uisettings"), parent, args )
{
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget(parent);
    m_uiconfigUi = new Ui::UiConfig();
    m_uiconfigUi->setupUi( w );
    l->addWidget( w );

    addConfig( UiConfig::self(), w );
    load();
}

UiPreferences::~UiPreferences()
{
    delete m_uiconfigUi;
}

void UiPreferences::save()
{
    KCModule::save();

    UiController *uiController = Core::self()->uiControllerInternal();
    foreach (Sublime::MainWindow *window, uiController->mainWindows())
        (static_cast<KDevelop::MainWindow*>(window))->loadSettings();
    uiController->loadSettings();
}

#include "uipreferences.moc"
