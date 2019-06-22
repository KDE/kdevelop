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

#include <KPluginSelector>
#include <KPluginInfo>

#include <interfaces/isession.h>

#include "../core.h"
#include "../plugincontroller.h"
#include "debug.h"


namespace KDevelop
{

PluginPreferences::PluginPreferences(QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
{
    auto* lay = new QVBoxLayout(this );
    lay->setMargin(0);
    selector = new KPluginSelector( this );
    lay->addWidget( selector );
    QMap<QString, QList<KPluginInfo>> plugins;
    const QMap<QString, QString> categories = {
        { QStringLiteral("Core"),               i18nc("@title:group", "Core") },
        { QStringLiteral("Project Management"), i18nc("@title:group", "Project Management") },
        { QStringLiteral("Version Control"),    i18nc("@title:group", "Version Control") },
        { QStringLiteral("Utilities"),          i18nc("@title:group", "Utilities") },
        { QStringLiteral("Documentation"),      i18nc("@title:group", "Documentation") },
        { QStringLiteral("Language Support"),   i18nc("@title:group", "Language Support") },
        { QStringLiteral("Debugging"),          i18nc("@title:group", "Debugging") },
        { QStringLiteral("Testing"),            i18nc("@title:group", "Testing") },
        { QStringLiteral("Analyzers"),          i18nc("@title:group", "Analyzers") },
        { QStringLiteral("Runtimes"),           i18nc("@title:group", "Runtimes") },
        { QStringLiteral("Other"),              i18nc("@title:group", "Other") }
    };
    const auto pluginInfos = Core::self()->pluginControllerInternal()->allPluginInfos();
    for (const KPluginMetaData& info : pluginInfos) {
        const QString loadMode = info.value(QStringLiteral("X-KDevelop-LoadMode"));
        if( loadMode.isEmpty() || loadMode == QLatin1String("UserSelectable") )
        {
            QString category = info.category();
            if (!categories.contains(category)) {
                if (!category.isEmpty()) {
                    qCWarning(SHELL) << "unknown category for plugin" << info.name() << ":" << info.category();
                }
                category = QStringLiteral("Other");
            }
            KPluginInfo kpi(info);
            plugins[category] << kpi;
        } else
            qCDebug(SHELL) << "skipping..." << info.pluginId() << info.value(QStringLiteral("X-KDevelop-Category")) << loadMode;
    }

    for (auto it = plugins.constBegin(), end = plugins.constEnd(); it != end; ++it) {
        selector->addPlugins(it.value(), KPluginSelector::ReadConfigFile,
                             categories.value(it.key()),
                             // no filter by category key, we did it ourselves above & will not work with "Other"
                             QString(),
                             Core::self()->activeSession()->config());
    }
    connect(selector, &KPluginSelector::changed, this, &PluginPreferences::changed);
}

void PluginPreferences::defaults()
{
    Core::self()->pluginControllerInternal()->resetToDefaults();
    selector->load();
}

void PluginPreferences::apply()
{
    selector->save();
    qCDebug(SHELL) << "Plugins before apply: " << Core::self()->pluginControllerInternal()->allPluginNames();
    Core::self()->pluginControllerInternal()->updateLoadedPlugins();
    qCDebug(SHELL) << "Plugins after apply: " << Core::self()->pluginControllerInternal()->allPluginNames();
    selector->load(); // Some plugins may have failed to load, they must be unchecked.
}

void PluginPreferences::reset()
{
    selector->load();
}

}


