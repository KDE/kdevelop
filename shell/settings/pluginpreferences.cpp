/* KDevelop Project Settings
 *
 * Copyright 2008 Andreas Pakulat <apaku@gmx.de>
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
#include "pluginpreferences.h"

#include <QVBoxLayout>

#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kpluginselector.h>
#include <kplugininfo.h>
#include <ksettings/dispatcher.h>
#include <kcomponentdata.h> // kdelibs4support

#include <interfaces/isession.h>

#include "../core.h"
#include "../plugincontroller.h"


namespace KDevelop
{

K_PLUGIN_FACTORY_WITH_JSON(PluginPreferencesFactory, "kcm_kdev_pluginsettings.json", registerPlugin<PluginPreferences>();)

PluginPreferences::PluginPreferences( QWidget *parent, const QVariantList &args )
    : KCModule( KAboutData::pluginData("kcm_kdev_pluginsettings"), parent, args )
{
    QVBoxLayout* lay = new QVBoxLayout(this );
    selector = new KPluginSelector( this );
    lay->addWidget( selector );
    QMap<QString, QList<KPluginInfo> > plugins;
    QMap<QString, QString> categories;
    categories["Core"] = i18nc("@title:group", "Core");
    categories["Project Management"] = i18nc("@title:group", "Project Management");
    categories["Version Control"] = i18nc("@title:group", "Version Control");
    categories["Utilities"] = i18nc("@title:group", "Utilities");
    categories["Documentation"] = i18nc("@title:group", "Documentation");
    categories["Language Support"] = i18nc("@title:group", "Language Support");
    categories["Debugging"] = i18nc("@title:group", "Debugging");
    categories["Testing"] = i18nc("@title:group", "Testing");
    categories["Other"] = i18nc("@title:group", "Other");
    foreach( const KPluginInfo& info, Core::self()->pluginControllerInternal()->allPluginInfos() )
    {
        QString loadMode = info.property("X-KDevelop-LoadMode").toString();
        if( info.property("X-KDevelop-Category") == "Global"
            && ( loadMode.isEmpty() || loadMode == "UserSelectable") )
        {
            QString category = info.category();
            if (!categories.contains(category)) {
                if (!category.isEmpty()) {
                    qWarning() << "unknown category for plugin" << info.name() << ":" << info.category();
                }
                category = "Other";
            }
            plugins[category] << info;
        }
    }
    QMap< QString, QList<KPluginInfo> >::const_iterator it = plugins.constBegin();
    QMap< QString, QList<KPluginInfo> >::const_iterator end = plugins.constEnd();
    while(it != end) {
        selector->addPlugins( it.value(), KPluginSelector::ReadConfigFile,
                              categories.value(it.key()), it.key(),
                              Core::self()->activeSession()->config() );
        ++it;
    }
    connect( selector, SIGNAL(changed(bool)), this, SLOT(changed()) );
    connect( selector, SIGNAL(configCommitted(QByteArray)), this, SLOT(reparseConfig(QByteArray)) );
    selector->load();
}

void PluginPreferences::reparseConfig( const QByteArray& conf )
{
    KSettings::Dispatcher::reparseConfiguration( conf );
}

void PluginPreferences::defaults()
{
    Core::self()->pluginControllerInternal()->resetToDefaults();
    selector->load();
    KCModule::defaults();
}

void PluginPreferences::save()
{
    selector->save();
    KCModule::save();
    Core::self()->pluginControllerInternal()->updateLoadedPlugins();
    selector->load();   // Some plugins may have failed to load, they must be unchecked.
}

void PluginPreferences::load()
{
    selector->load();
    KCModule::load();
}

}

#include "pluginpreferences.moc"

